#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "spdlog/logger.h"

#include "nhope/async/ao-context.h"

#include "royalbed/detail/request.h"
#include "royalbed/detail/responce.h"

namespace royalbed::detail {

class Router;
using RawPathParams = std::vector<std::pair<std::string, std::string>>;

struct RequestContext final
{
    const std::uint64_t num;

    nhope::AOContext aoCtx;
    std::shared_ptr<spdlog::logger> log;

    const Router& router;

    RequestPtr request;
    RawPathParams rawPathParams;

    ResponcePtr responce;
};

}   // namespace royalbed::detail
