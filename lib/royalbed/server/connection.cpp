#include <cassert>
#include <cstdint>
#include <memory>
#include <optional>

#include "spdlog/logger.h"

#include "nhope/async/ao-context-close-handler.h"
#include "nhope/async/ao-context.h"
#include "nhope/io/io-device.h"
#include "nhope/io/pushback-reader.h"

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
      , m_ctx(params.ctx)
      , m_sock(std::move(params.sock))
      , m_aoCtx(parent)
    {
        m_sessionIn = nhope::PushbackReader::create(m_aoCtx, *m_sock);

        this->startSession();
        m_aoCtx.addCloseHandler(*this);
    }

private:
    ~Connection()
    {
        m_aoCtx.removeCloseHandler(*this);
    }

    void aoContextClose() noexcept override
    {
        m_ctx.connectionClosed(m_num);
        delete this;
    }

    Router& router() noexcept override
    {
        return m_ctx.router();
    }

    void sessionReceivedRequest(std::uint32_t /*sessionNum*/) noexcept override
    {}

    void sessionFinished(std::uint32_t sessionNum, bool /*success*/) noexcept override
    {
        m_ctx.sessionFinished(sessionNum);
        m_aoCtx.close();
    }

    void startSession()
    {
        auto [sessionNum, sessionLog] = m_ctx.startSession(m_num);
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
    ConnectionCtx& m_ctx;

    nhope::IODevicePtr m_sock;
    nhope::PushbackReaderPtr m_sessionIn;

    nhope::AOContext m_aoCtx;
};

}   // namespace

void openConnection(nhope::AOContext& aoCtx, ConnectionParams&& params)
{
    new Connection(aoCtx, std::move(params));
}

}   // namespace royalbed::server::detail
