#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <memory>
#include <span>
#include <string>
#include <utility>
#include <vector>

#include "3rdparty/llhttp/llhttp.h"

#include "nhope/async/ao-context-error.h"
#include "nhope/async/ao-context.h"
#include "nhope/async/future.h"
#include "nhope/async/safe-callback.h"
#include "nhope/io/pushback-reader.h"

#include "royalbed/body-reader.h"
#include "royalbed/detail/request.h"
#include "receive-request.h"

#include "royalbed/detail/uri.h"
#include "royalbed/http-error.h"
#include "royalbed/http-status.h"

namespace royalbed::detail {
namespace {

constexpr std::size_t receiveBufSize = 4096;

class RequestReceiver final : public std::enable_shared_from_this<RequestReceiver>
{
public:
    RequestReceiver(nhope::AOContext& aoCtx, nhope::PushbackReader& device)
      : m_aoCtx(aoCtx)
      , m_device(device)
      , m_httpParser(std::make_unique<llhttp_t>())
    {
        llhttp_init(m_httpParser.get(), HTTP_REQUEST, &llhttpSettings);
        m_httpParser->data = this;
    }

    ~RequestReceiver()
    {
        if (!m_promise.satisfied()) {
            m_promise.setException(std::make_exception_ptr(nhope::AsyncOperationWasCancelled()));
        }
    }

    nhope::Future<RequestPtr> start()
    {
        this->readNextPortion();
        return m_promise.future();
    }

private:
    bool processData(std::span<std::uint8_t> data)
    {
        assert(!m_headersComplete);   // NOLINT

        if (!data.empty()) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
            llhttp_execute(m_httpParser.get(), reinterpret_cast<char*>(data.data()), data.size());
        } else {
            llhttp_finish(m_httpParser.get());
        }

        if (m_httpParser->error == HPE_OK) {
            return true;
        }

        if (m_httpParser->error != HPE_PAUSED) {
            const auto* reason = llhttp_get_error_reason(m_httpParser.get());
            auto ex = std::make_exception_ptr(HttpError(HttpStatus::BadRequest, reason));
            m_promise.setException(std::move(ex));
            return false;
        }

        // Headers received

        assert(m_headersComplete);   // NOLINT

        m_httpParser->data = nullptr;
        llhttp_resume(m_httpParser.get());

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        const auto* beginBody = reinterpret_cast<const std::uint8_t*>(llhttp_get_error_pos(m_httpParser.get()));
        m_device.unread({std::to_address(beginBody), std::to_address(data.end())});
        m_request->method = llhttp_method_name(static_cast<llhttp_method_t>(m_httpParser->method));
        m_request->body = BodyReader::create(m_aoCtx, m_device, std::move(m_httpParser));

        m_promise.setValue(std::move(m_request));

        return false;
    }

    void readNextPortion()
    {
        auto safeHandler =
          nhope::makeSafeCallback(m_aoCtx, [self = shared_from_this()](std::exception_ptr err, std::size_t n) {
              if (err) {
                  self->m_promise.setException(std::move(err));
                  return;
              }

              if (n == 0) {
                  auto ex = std::make_exception_ptr(HttpError(HttpStatus::BadRequest));
                  self->m_promise.setException(std::move(ex));
              }

              if (self->processData(std::span(self->m_receiveBuf.begin(), n))) {
                  self->readNextPortion();
              }
          });

        m_device.read(m_receiveBuf, std::move(safeHandler));
    }

    static int onUrl(llhttp_t* httpParser, const char* at, std::size_t size)
    {
        auto* self = static_cast<RequestReceiver*>(httpParser->data);
        self->m_url.append(at, size);
        return HPE_OK;
    }

    static int onUrlComplete(llhttp_t* httpParser)
    {
        try {
            auto* self = static_cast<RequestReceiver*>(httpParser->data);
            self->m_request->uri = Uri::parseRelative(self->m_url);
            return HPE_OK;
        } catch (UriParseError&) {
            return HPE_INVALID_URL;
        }
    }

    static int onHeaderName(llhttp_t* httpParser, const char* at, std::size_t size)
    {
        auto* self = static_cast<RequestReceiver*>(httpParser->data);
        self->m_curHeaderName.append(at, size);
        return HPE_OK;
    }

    static int onHeaderValue(llhttp_t* httpParser, const char* at, std::size_t size)
    {
        auto* self = static_cast<RequestReceiver*>(httpParser->data);
        self->m_curHeaderValue.append(at, size);
        return HPE_OK;
    }

    static int onHeaderComplete(llhttp_t* httpParser)
    {
        auto* self = static_cast<RequestReceiver*>(httpParser->data);

        assert(self->m_curHeaderName.size() > 0);   // NOLINT

        self->m_request->headers[self->m_curHeaderName] = self->m_curHeaderValue;
        self->m_curHeaderName.clear();
        self->m_curHeaderValue.clear();

        return HPE_OK;
    }

    static int onHeadersComplete(llhttp_t* httpParser)
    {
        auto* self = static_cast<RequestReceiver*>(httpParser->data);
        self->m_headersComplete = true;
        return HPE_PAUSED;
    }

    static constexpr llhttp_settings_s llhttpSettings = {
      .on_url = onUrl,
      .on_header_field = onHeaderName,
      .on_header_value = onHeaderValue,

      .on_headers_complete = onHeadersComplete,
      .on_url_complete = onUrlComplete,
      .on_header_value_complete = onHeaderComplete,
    };

    nhope::AOContextRef m_aoCtx;
    nhope::PushbackReader& m_device;

    nhope::Promise<RequestPtr> m_promise;

    std::unique_ptr<llhttp_t> m_httpParser;
    std::string m_url;
    std::string m_curHeaderName;
    std::string m_curHeaderValue;
    bool m_headersComplete = false;

    std::array<std::uint8_t, receiveBufSize> m_receiveBuf{};

    RequestPtr m_request = Request::create();
};

}   // namespace

nhope::Future<RequestPtr> receiveRequest(nhope::AOContext& aoCtx, nhope::PushbackReader& device)
{
    auto receiver = std::make_shared<RequestReceiver>(aoCtx, device);
    return receiver->start();
}

}   // namespace royalbed::detail
