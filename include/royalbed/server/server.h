#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "nhope/io/sock-addr.h"
#include "spdlog/logger.h"
#include "nhope/async/ao-context.h"

#include "royalbed/server/router.h"

namespace royalbed::server {

struct ServerParams
{
    std::string bindAddress;
    std::uint16_t port;

    Router router;

    std::shared_ptr<spdlog::logger> log;
};

class Server;
using ServerPtr = std::unique_ptr<Server>;

class Server
{
public:
    virtual ~Server() = default;

    [[nodiscard]] virtual nhope::SockAddr bindAddress() const = 0;

    static ServerPtr start(nhope::AOContext& aoCtx, ServerParams&& params);
};

}   // namespace royalbed::server
