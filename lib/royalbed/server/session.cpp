#include <array>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include "spdlog/logger.h"

#include "nhope/async/ao-context.h"
#include "nhope/async/future.h"
#include "nhope/io/io-device.h"
#include "nhope/io/pushback-reader.h"

#include "royalbed/common/detail/uptime.h"
#include "royalbed/server/detail/receive-request.h"
#include "royalbed/server/detail/send-response.h"
#include "royalbed/server/detail/session.h"
#include "royalbed/server/error.h"
#include "royalbed/server/http-status.h"
#include "royalbed/server/low-level-handler.h"
#include "royalbed/server/middleware.h"
#include "royalbed/server/request-context.h"
#include "royalbed/server/request.h"
#include "royalbed/server/response.h"

namespace royalbed::server::detail {

namespace {
using namespace std::literals;

const auto ConnectionHeader = "Connection"s;
const auto ConnectionHeaderCloseValue = "close"s;

std::string gmtDateTime()
{
    constexpr auto bufSize{100};
    std::array<char, bufSize> dateBuffer{};
    const auto curTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    const char* fmt = "%a, %d %Y %b %H:%M:%S GMT";
    const auto count = std::strftime(dateBuffer.data(), bufSize, fmt, std::gmtime(&curTime));
    return {dateBuffer.data(), count};
}

template<typename AsyncFunc>
auto safeCall(RequestContext& ctx, AsyncFunc&& func)
{
    using Future = std::invoke_result_t<AsyncFunc, RequestContext&>;
    using Promise = nhope::Promise<typename Future::Type>;

    try {
        return func(ctx);
    } catch (...) {
        Promise p;
        p.setException(std::current_exception());
        return p.future();
    }
}

class Session final : public nhope::AOContextCloseHandler
{
public:
    Session(nhope::AOContext& aoCtx, SessionParams&& param)
      : m_num(param.num)
      , m_ctx(param.ctx)
      , m_in(param.in)
      , m_out(param.out)      
      , m_requestCtx{
          .num = param.num,
          .log = std::move(param.log),
          .router = param.ctx.router(),
          .request{},
          .rawPathParams{},
          .response{},
          .aoCtx = nhope::AOContext(aoCtx),
        }
        , m_upTime(m_requestCtx.log, "session time:")
    {
        m_requestCtx.aoCtx.startCancellableTask(
          [this] {
              this->start();
          },
          *this);
    }

private:
    ~Session() override
    {
        assert(m_finished);   // NOLINT
        m_requestCtx.aoCtx.removeCloseHandler(*this);
    }

    void aoContextClose() noexcept override
    {
        if (!m_finished) {
            m_requestCtx.log->info("session was cancelled");
            this->finished(false);
        }

        delete this;
    }

    void start()
    {
        receiveRequest(m_requestCtx.aoCtx, m_in)
          .then(aoCtx(),
                [this](auto req) mutable {
                    m_ctx.sessionReceivedRequest(m_num);
                    return this->processingRequest(std::move(req));
                })
          .fail(aoCtx(),
                [this](auto ex) {
                    return this->makeResponseFromError(std::move(ex));
                })
          .then(aoCtx(),
                [this] {
                    return this->sendResponse();
                })
          .then(aoCtx(),
                [this](bool keepAlive) {
                    this->finished(keepAlive);
                })
          .fail(aoCtx(), [this](auto ex) {
              try {
                  std::rethrow_exception(std::move(ex));
              } catch (const std::exception& e) {
                  m_requestCtx.log->error("session failed: {}", e.what());
              }
              this->finished(false);
          });
    }

    nhope::Future<void> processingRequest(Request&& req)
    {
        m_requestCtx.log->trace("request: \"{} {}\"", req.method, req.uri.path);

        auto routeResult = m_requestCtx.router.route(req.method, req.uri.path);
        m_handler = std::move(routeResult.handler);
        m_middlewares = std::move(routeResult.middlewares);
        m_requestCtx.rawPathParams = std::move(routeResult.rawPathParams);
        m_requestCtx.request = std::move(req);

        return this->doMiddlewares().then(aoCtx(), [this](bool doHandler) {
            if (!doHandler) {
                return nhope::makeReadyFuture();
            }

            return safeCall(m_requestCtx, m_handler);
        });
    }

    nhope::Future<bool> doMiddlewares()
    {
        if (m_middlewares.empty()) {
            return nhope::makeReadyFuture<bool>(true);
        }

        const auto& middleware = m_middlewares.front();
        return safeCall(m_requestCtx, middleware).then(aoCtx(), [this](bool doNext) {
            assert(!m_middlewares.empty());   // NOLINT

            m_middlewares.pop_front();
            if (!doNext) {
                return nhope::makeReadyFuture<bool>(false);
            }
            return this->doMiddlewares();
        });
    }

    void makeResponseFromError(std::exception_ptr ex)
    {
        try {
            std::rethrow_exception(std::move(ex));
        } catch (const HttpError& e) {
            m_requestCtx.response = common::makePlainTextResponse(aoCtx(), e.httpStatus(), e.what());

        } catch (const std::exception& e) {
            m_requestCtx.response = common::makePlainTextResponse(aoCtx(), HttpStatus::InternalServerError, e.what());
        }
    }

    bool needClose() const noexcept
    {
        if (m_ctx.sessionNeedClose()) {
            return true;
        }
        if (auto it = m_requestCtx.request.headers.find(ConnectionHeader); it != m_requestCtx.request.headers.end()) {
            return it->second == ConnectionHeaderCloseValue;
        }
        return false;
    }

    nhope::Future<bool> sendResponse()
    {
        m_requestCtx.log->trace("response: {}", m_requestCtx.response.status);
        auto needAddClose = needClose();
        if (needAddClose) {
            m_requestCtx.response.headers[ConnectionHeader] = ConnectionHeaderCloseValue;
        }
        m_requestCtx.response.headers["Date"] = gmtDateTime();

        return detail::sendResponse(aoCtx(), std::move(m_requestCtx.response), m_out)
          .then(aoCtx(), [this, keepAlive = !needAddClose](auto size) {
              m_requestCtx.log->trace("response has been sent: {} bytes", size);
              return keepAlive;
          });
    }

    void finished(bool keepAlive)
    {
        assert(!m_finished);   // NOLINT

        m_finished = true;

        const auto num = m_num;
        auto& ctx = m_ctx;

        m_requestCtx.aoCtx.close();

        ctx.sessionFinished(num, keepAlive);
    }

    nhope::AOContext& aoCtx()
    {
        return m_requestCtx.aoCtx;
    }

    const std::uint32_t m_num;
    SessionCtx& m_ctx;

    nhope::PushbackReader& m_in;
    nhope::Writter& m_out;

    bool m_finished = false;

    LowLevelHandler m_handler;
    std::list<Middleware> m_middlewares;

    RequestContext m_requestCtx;
    royalbed::common::detail::UpTimeLogger m_upTime;
};

}   // namespace

void startSession(nhope::AOContext& aoCtx, SessionParams&& param)
{
    new Session(aoCtx, std::move(param));
}

}   // namespace royalbed::server::detail
