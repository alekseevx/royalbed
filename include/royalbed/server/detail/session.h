#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "spdlog/logger.h"

#include "nhope/async/ao-context-close-handler.h"
#include "nhope/async/ao-context.h"
#include "nhope/io/io-device.h"
#include "nhope/io/pushback-reader.h"

#include "royalbed/server/request-context.h"
#include "royalbed/server/router.h"

namespace royalbed::server::detail {

class SessionCtx
{
public:
    virtual Router& router() noexcept = 0;

    virtual void sessionReceivedRequest(std::uint32_t sessionNum) noexcept = 0;
    virtual void sessionFinished(std::uint32_t sessionNum, bool success) noexcept = 0;
};

struct SessionParams
{
    std::uint32_t num;
    SessionCtx& ctx;

    nhope::PushbackReader& in;
    nhope::Writter& out;
    std::shared_ptr<spdlog::logger> log;
};

void startSession(nhope::AOContext& aoCtx, SessionParams&& params);

}   // namespace royalbed::server::detail
