#pragma once

#include <cstdint>
#include <memory>
#include <string>

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

void start(nhope::AOContext& aoCtx, ServerParams&& params);

}   // namespace royalbed::server
