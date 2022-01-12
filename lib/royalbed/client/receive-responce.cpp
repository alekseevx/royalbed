#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <memory>
#include <span>
#include <string>

#include "nhope/async/ao-context-error.h"
#include "nhope/async/ao-context.h"
#include "nhope/async/future.h"
#include "nhope/async/safe-callback.h"
#include "nhope/io/pushback-reader.h"

#include "3rdparty/llhttp/llhttp.h"
#include "royalbed/common/detail/body-reader.h"

#include "royalbed/client/responce.h"
#include "royalbed/client/http-error.h"
#include "royalbed/client/http-status.h"
#include "royalbed/client/detail/receive-responce.h"

namespace royalbed::client::detail {
namespace {

using common::detail::BodyReader;
using common::detail::BodyReaderPtr;

constexpr std::size_t receiveBufSize = 4096;

// FIXME: Duplicates RequestReceiver
class ResponceReceiver final : public std::enable_shared_from_this<ResponceReceiver>
{
public:
    ResponceReceiver(nhope::AOContext& aoCtx, nhope::PushbackReader& device)
      : m_aoCtx(aoCtx)
      , m_device(device)
      , m_httpParser(std::make_unique<llhttp_t>())
    {
        llhttp_init(m_httpParser.get(), HTTP_RESPONSE, &llhttpSettings);
        m_httpParser->data = this;
    }

    ~ResponceReceiver()
    {
        if (!m_promise.satisfied()) {
            m_promise.setException(std::make_exception_ptr(nhope::AsyncOperationWasCancelled()));
        }
    }

    nhope::Future<Responce> start()
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
            // FIXME: Need more suitable error
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
        m_responce.body = BodyReader::create(m_aoCtx, m_device, std::move(m_httpParser));

        m_promise.setValue(std::move(m_responce));

        return false;
    }

    void readNextPortion()
    {
        m_device.read(m_receiveBuf, [self = shared_from_this()](std::exception_ptr err, std::size_t n) {
            self->m_aoCtx.exec([self, err = std::move(err), n] {
                if (!self->processData(std::span(self->m_receiveBuf.begin(), n))) {
                    return;
                }

                if (err) {
                    self->m_promise.setException(err);
                    return;
                }

                if (n == 0) {
                    // FIXME: EOF? Need more suitable error
                    auto ex = std::make_exception_ptr(HttpError(HttpStatus::BadRequest));
                    self->m_promise.setException(std::move(ex));
                    return;
                }

                self->readNextPortion();
            });
        });
    }

    static int onStatus(llhttp_t* httpParser, const char* at, std::size_t size)
    {
        auto* self = static_cast<ResponceReceiver*>(httpParser->data);
        self->m_status.append(at, size);
        return HPE_OK;
    }

    static int onStatusComplete(llhttp_t* httpParser)
    {
        auto* self = static_cast<ResponceReceiver*>(httpParser->data);
        self->m_responce.status = httpParser->status_code;
        self->m_responce.statusMessage = std::move(self->m_status);
        return HPE_OK;
    }

    static int onHeaderName(llhttp_t* httpParser, const char* at, std::size_t size)
    {
        auto* self = static_cast<ResponceReceiver*>(httpParser->data);
        self->m_curHeaderName.append(at, size);
        return HPE_OK;
    }

    static int onHeaderValue(llhttp_t* httpParser, const char* at, std::size_t size)
    {
        auto* self = static_cast<ResponceReceiver*>(httpParser->data);
        self->m_curHeaderValue.append(at, size);
        return HPE_OK;
    }

    static int onHeaderComplete(llhttp_t* httpParser)
    {
        auto* self = static_cast<ResponceReceiver*>(httpParser->data);

        assert(self->m_curHeaderName.size() > 0);   // NOLINT

        self->m_responce.headers[self->m_curHeaderName] = self->m_curHeaderValue;
        self->m_curHeaderName.clear();
        self->m_curHeaderValue.clear();

        return HPE_OK;
    }

    static int onHeadersComplete(llhttp_t* httpParser)
    {
        auto* self = static_cast<ResponceReceiver*>(httpParser->data);
        self->m_headersComplete = true;
        return HPE_PAUSED;
    }

    static constexpr llhttp_settings_s llhttpSettings = {
      .on_status = onStatus,
      .on_header_field = onHeaderName,
      .on_header_value = onHeaderValue,

      .on_headers_complete = onHeadersComplete,
      .on_status_complete = onStatusComplete,
      .on_header_value_complete = onHeaderComplete,
    };

    nhope::AOContextRef m_aoCtx;
    nhope::PushbackReader& m_device;

    nhope::Promise<Responce> m_promise;

    std::unique_ptr<llhttp_t> m_httpParser;
    std::string m_status;
    std::string m_curHeaderName;
    std::string m_curHeaderValue;
    bool m_headersComplete = false;

    std::array<std::uint8_t, receiveBufSize> m_receiveBuf{};

    Responce m_responce;
};

}   // namespace

nhope::Future<Responce> receiveResponce(nhope::AOContext& aoCtx, nhope::PushbackReader& device)
{
    auto receiver = std::make_shared<ResponceReceiver>(aoCtx, device);
    return receiver->start();
}

}   // namespace royalbed::client::detail
