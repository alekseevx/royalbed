#include <cassert>
#include <memory>
#include <string>
#include <string_view>

#include "fmt/core.h"
#include "nhope/async/ao-context.h"
#include "nhope/io/tcp.h"

#include "royalbed/common/detail/uptime.h"
#include "royalbed/server/detail/connection.h"
#include "royalbed/server/server.h"

namespace royalbed::server {
namespace {
using namespace detail;

nhope::TcpServerPtr startTcpServer(nhope::AOContext& aoCtx, const ServerParams& params)
{
    return nhope::TcpServer::start(aoCtx, {
                                            .address = params.bindAddress,
                                            .port = params.port,
                                          });
}

class ServerImpl final
  : public Server
  , public detail::ConnectionCtx
{
public:
    ServerImpl(nhope::AOContext& aoCtx, ServerParams&& params)
      : m_log(params.log)
      , m_tcpServer(startTcpServer(aoCtx, params))
      , m_router(std::move(params.router))
      , m_upTime(m_log, "service uptime")
      , m_aoCtx(aoCtx)
    {
        const auto bindAddr = m_tcpServer->bindAddress();
        m_log->info("service accepting HTTP connections at http://{}", bindAddr.toString());

        for (const auto& resource : m_router.resources()) {
            m_log->info("resource published on route {}", resource);
        }

        m_aoCtx.exec([this] {
            this->acceptNextConnection();
        });
    }

    ~ServerImpl() override
    {
        m_aoCtx.close();
    }

    [[nodiscard]] nhope::SockAddr bindAddress() const override
    {
        return m_tcpServer->bindAddress();
    }

private:
    [[nodiscard]] const Router& router() const noexcept override
    {
        return m_router;
    }

    SessionAttr startSession(std::uint32_t /*connectionNum*/) override
    {
        assert(m_aoCtx.workInThisThread());   // NOLINT
        const auto sessionNum = ++m_sessionCounter;
        ++m_activeSessionCount;

        return {
          .num = sessionNum,
          .log = m_log->clone(fmt::format("{}/S{}", m_log->name(), sessionNum)),
        };
    }

    void sessionFinished(std::uint32_t /*sessionNum*/) override
    {
        assert(m_aoCtx.workInThisThread() || !m_aoCtx.isOpen());   // NOLINT
        --m_activeSessionCount;
    }

    void connectionClosed(std::uint32_t connectionNum) override
    {
        assert(m_aoCtx.workInThisThread() || !m_aoCtx.isOpen());   // NOLINT

        --m_activeConnectionCount;
        m_log->trace("The connection with num={} closed", connectionNum);
    }

    void acceptNextConnection()
    {
        m_tcpServer->accept().then(m_aoCtx, [this](auto connection) {
            ++m_activeConnectionCount;
            const auto connectionNum = ++m_connectionCounter;

            m_log->trace("New connection accepted: num={}, peer={}", connectionNum,
                         connection->peerAddress().toString());

            detail::openConnection(m_aoCtx, {
                                              .num = connectionNum,
                                              .keepAlive = m_keepAlive,
                                              .ctx = *this,
                                              .log = m_log->clone(fmt::format("{}/C{}", m_log->name(), connectionNum)),
                                              .sock = std::move(connection),
                                            });

            this->acceptNextConnection();
        });
    }

    std::shared_ptr<spdlog::logger> m_log;
    nhope::TcpServerPtr m_tcpServer;
    Router m_router;

    // TODO add max connections limit
    KeepAliveParams m_keepAlive{};

    std::uint32_t m_activeConnectionCount = 0;
    std::uint32_t m_activeSessionCount = 0;

    std::uint32_t m_connectionCounter = 0;
    std::uint32_t m_sessionCounter = 0;

    royalbed::common::detail::UpTimeLogger m_upTime;

    nhope::AOContext m_aoCtx;
};

}   // namespace

ServerPtr Server::start(nhope::AOContext& aoCtx, ServerParams&& params)
{
    return std::make_unique<ServerImpl>(aoCtx, std::move(params));
}

}   // namespace royalbed::server
