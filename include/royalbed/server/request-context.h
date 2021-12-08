#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "spdlog/logger.h"

#include "nhope/async/ao-context.h"

#include "royalbed/server/request.h"
#include "royalbed/server/responce.h"

namespace royalbed::server {

class Router;
using RawPathParams = std::vector<std::pair<std::string, std::string>>;

struct RequestContext final
{
    const std::uint64_t num;

    std::shared_ptr<spdlog::logger> log;

    const Router& router;

    Request request;
    RawPathParams rawPathParams;

    Responce responce;

    nhope::AOContext aoCtx;
};

}   // namespace royalbed::server
