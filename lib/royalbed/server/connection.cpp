#include <cassert>
#include <chrono>
#include <cstdint>
#include <memory>
#include <optional>

#include "nhope/async/timer.h"
#include "nhope/io/tcp.h"
#include "royalbed/common/http-status.h"
#include "royalbed/common/response.h"
#include "royalbed/server/detail/send-response.h"
#include "spdlog/logger.h"

#include "nhope/async/ao-context-close-handler.h"
#include "nhope/async/ao-context.h"
#include "nhope/io/io-device.h"
#include "nhope/io/pushback-reader.h"

#include "royalbed/common/detail/uptime.h"
#include "royalbed/server/detail/session.h"
#include "royalbed/server/detail/connection.h"

namespace royalbed::server::detail {
namespace {

class Connection final
  : public nhope::AOContextCloseHandler
  , public SessionCtx
{
public:
    Connection(nhope::AOContext& parent, ConnectionParams&& params)
      : m_num(params.num)
      , m_log(std::move(params.log))
      , m_ctx(params.ctx)
      , m_sock(std::move(params.sock))
      , m_leftRequests(params.keepAlive.requestsCount > 0 ? params.keepAlive.requestsCount : 1)
      , m_upTime(m_log, "connection time:")
      , m_aoCtx(parent)
    {
        nhope::setTimeout(m_aoCtx, params.keepAlive.timeout, [this](auto) {
            processTimeout();
        });

        m_aoCtx.startCancellableTask(
          [this] {
              m_sessionIn = nhope::PushbackReader::create(m_aoCtx, *m_sock);
              this->startSession();
          },
          *this);
    }

private:
    ~Connection()
    {
        m_aoCtx.removeCloseHandler(*this);
    }

    void processTimeout()
    {
        m_leftRequests = 0;
        if (m_haveActiveSession) {
            return;
        }
        constexpr auto incomingRequestTimeout = std::chrono::seconds(2);
        // TODO use nhope::race (not implemented yet)
        nhope::setTimeout(m_aoCtx, incomingRequestTimeout, [this](auto) {
            m_aoCtx.close();
        });
        detail::sendResponse(m_aoCtx,
                             Response{
                               .status = HttpStatus::RequestTimeout,
                               .statusMessage = std::string(HttpStatus::message(HttpStatus::RequestTimeout)),
                               .headers = {{"Connection", "close"}},
                               .body = nullptr,
                             },
                             *m_sock)
          .then(m_aoCtx, [this](auto) {
              m_aoCtx.close();
          });
    }

    void aoContextClose() noexcept override
    {
        m_ctx.connectionClosed(m_num);
        m_log->trace("close");
        delete this;
    }

    [[nodiscard]] const Router& router() const noexcept override
    {
        return m_ctx.router();
    }

    void sessionReceivedRequest(std::uint32_t /*sessionNum*/) noexcept override
    {
        m_haveActiveSession = true;
    }

    bool sessionNeedClose() noexcept override
    {
        return m_leftRequests == 0;
    }

    void sessionFinished(std::uint32_t sessionNum, bool keepAlive) noexcept override
    {
        m_haveActiveSession = false;
        m_ctx.sessionFinished(sessionNum);
        m_log->trace("The session with num={} finished", sessionNum);

        if (!keepAlive) {
            m_leftRequests = 0;
        }

        if (m_leftRequests > 0) {
            startSession();
        } else {
            m_aoCtx.close();
        }
    }

    void startSession()
    {
        assert(m_leftRequests > 0);   // NOLINT
        --m_leftRequests;
        auto [sessionNum, sessionLog] = m_ctx.startSession(m_num);

        m_log->trace("Start a new session: num={}", sessionNum);
        detail::startSession(m_aoCtx, SessionParams{
                                        .num = sessionNum,
                                        .ctx = *this,
                                        .in = *m_sessionIn,
                                        .out = *m_sock,
                                        .log = std::move(sessionLog),
                                      });
    }

private:
    const std::uint32_t m_num;
    std::shared_ptr<spdlog::logger> m_log;
    ConnectionCtx& m_ctx;

    nhope::TcpSocketPtr m_sock;
    nhope::PushbackReaderPtr m_sessionIn;

    std::uint32_t m_leftRequests;
    bool m_haveActiveSession{};

    royalbed::common::detail::UpTimeLogger m_upTime;

    nhope::AOContext m_aoCtx;
};

}   // namespace

void openConnection(nhope::AOContext& aoCtx, ConnectionParams&& params)
{
    new Connection(aoCtx, std::move(params));
}

}   // namespace royalbed::server::detail
