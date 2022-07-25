#pragma once

#include <chrono>
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

struct KeepAliveParams
{
    // Ограничивает максимальное время, в течение которого могут обрабатываться запросы в рамках keep-alive соединения.
    // По достижении заданного времени соединение закрывается после обработки очередного запроса.
    std::chrono::seconds timeout{defaultMaxTime};

    // Задаёт максимальное число запросов, которые можно сделать по одному соединению.
    // После того, как сделано максимальное число запросов, соединение закрывается.
    // Для поддержки keep-alive соединений необходимо указать число больше 1
    std::uint16_t requestsCount{defaultRequests};

    static constexpr std::uint16_t defaultRequests{1000};
    static constexpr auto defaultMaxTime{std::chrono::seconds(600)};
};

struct ConnectionParams
{
    std::uint32_t num;
    KeepAliveParams keepAlive;
    ConnectionCtx& ctx;
    std::shared_ptr<spdlog::logger> log;
    nhope::TcpSocketPtr sock;
};

void openConnection(nhope::AOContext& aoCtx, ConnectionParams&& params);

}   // namespace royalbed::server::detail
