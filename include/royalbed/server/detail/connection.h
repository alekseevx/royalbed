#pragma once

#include <cstdint>
#include <optional>

#include "nhope/async/ao-context.h"
#include "nhope/io/io-device.h"
#include "nhope/io/tcp.h"
#include "spdlog/logger.h"

#include "royalbed/server/router.h"

namespace royalbed::server::detail {

struct SessionAttr
{
    std::uint32_t num;
    std::shared_ptr<spdlog::logger> log;
};

class ConnectionCtx
{
public:
    [[nodiscard]] virtual const Router& router() const noexcept = 0;

    virtual SessionAttr startSession(std::uint32_t connectionNum) = 0;
    virtual void sessionFinished(std::uint32_t sessionNum) = 0;
    virtual void connectionClosed(std::uint32_t connectionNum) = 0;
};

struct ConnectionParams
{
    std::uint32_t num;
    ConnectionCtx& ctx;
    std::shared_ptr<spdlog::logger> log;
    nhope::TcpSocketPtr sock;
};

void openConnection(nhope::AOContext& aoCtx, ConnectionParams&& params);

}   // namespace royalbed::server::detail
