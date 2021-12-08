#if 0
#include <cstdint>
#include <memory>
#include <utility>

#include "nhope/async/ao-context-close-handler.h"
#include "nhope/async/ao-context.h"
#include "nhope/io/tcp.h"

#include "royalbed/server/detail/receive-request.h"
#include "royalbed/server/router.h"
#include "royalbed/server/server.h"
#include "spdlog/logger.h"

namespace royalbed::server {
namespace {

nhope::TcpServerPtr startTcpServer(nhope::AOContext& aoCtx, const ServerParam& params)
{
    return nhope::TcpServer::start(aoCtx, {
                                            .address = params.bindAddress,
                                            .port = params.port,
                                          });
}

class Server final : public nhope::AOContextCloseHandler
{
public:
    Server(nhope::AOContext& aoCtx, ServerParam&& params);
    ~Server();

    nhope::AOContext& aoCtx();

    int makeConnectionId();
    void connectionClosed();

    int makeSessionId();
    void sessionFinished();

    void aoContextClose() noexcept override;

private:
    void acceptNextConnection();

    nhope::AOContextRef m_aoCtx;

    std::shared_ptr<spdlog::logger> m_log;
    nhope::TcpServerPtr m_tcpServer;
    Router m_router;

    int m_activeConnectionCount = 0;
    int m_activeSeesionCount = 0;

    int m_connectionCounter = 0;
    int m_seesionCounter = 0;
};

class Connection final : public nhope::AOContextCloseHandler
{
public:
    Connection(Server& server, nhope::TcpSocketPtr connection);
    ~Connection();

    void sessionFinished();

    void aoContextClose() noexcept override;

private:
    int m_num;

    nhope::TcpSocketPtr m_connection;

    Server& m_server;
};

class Session final : public nhope::AOContextCloseHandler
{
public:
private:
    nhope::AOContext m_aoCtx;
    Connection& m_connection;
};

Connection::Connection()

  Server::Server(nhope::AOContext& aoCtx, ServerParam&& params)
  : m_aoCtx(aoCtx)
  , m_log(std::move(params.log))
  , m_tcpServer(startTcpServer(aoCtx, params))
  , m_router(std::move(params.router))
{
    this->acceptNextConnection();
    m_aoCtx.addCloseHandler(*this);
}

Server::~Server()
{
    m_aoCtx.removeCloseHandler(*this);
}

void Server::aoContextClose() noexcept
{
    delete this;
}

void Server::acceptNextConnection()
{
    m_tcpServer->accept().then([this](auto connection) {
        new Connection(*this, std::move(connection));
        this->acceptNextConnection();
    });
}

}   // namespace

void listen(nhope::AOContext& aoCtx, ServerParam&& params)
{
    new Server(aoCtx, std::move(params));
}

void f()
{
    nhope::AOContext ctx;
    listen(ctx);

    ctx.close()
}

}   // namespace royalbed::server
#endif