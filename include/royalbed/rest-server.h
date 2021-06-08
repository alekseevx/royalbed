#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string_view>
#include <string>

#include <spdlog/logger.h>
#include <corvusoft/restbed/service.hpp>
#include <royalbed/router.h>

namespace royalbed {
using namespace std::chrono_literals;

static constexpr auto defaultConnectionTimeout = 5000ms;

class RestServer final
{
public:
    enum class State
    {
        Running,
        Stopping,
        Stopped
    };

    explicit RestServer();
    ~RestServer();

    RestServer& setLogger(std::shared_ptr<spdlog::logger> logger);
    RestServer& setConnectionTimeout(std::chrono::milliseconds timeout);
    RestServer& setKeepAlive(bool keepAlive);
    RestServer& setThreadCount(std::size_t threadCount);
    RestServer& setBindAddress(std::string_view bindAddress);
    RestServer& setPort(std::uint16_t port);
    RestServer& setReuseAddress(bool reuseAddress);
    RestServer& setRouter(const Router& router);

    void listen();
    void stop();

private:
    [[nodiscard]] std::shared_ptr<restbed::Settings> makeRestbedSettings() const;
    [[nodiscard]] std::unique_ptr<restbed::Service> makeEngine() const;

    static constexpr std::uint16_t invalidPort = 0xFFFF;

    std::mutex m_mutex;
    State m_state = State::Stopped;
    std::shared_ptr<spdlog::logger> m_logger;

    std::chrono::milliseconds m_connectionTimeout = defaultConnectionTimeout;
    bool m_keepAlive = true;
    std::size_t m_threadCount;
    std::string m_bindAddress;
    std::uint16_t m_port = invalidPort;
    bool m_reuseAddress = false;
    Router m_router;

    std::unique_ptr<restbed::Service> m_engine;
};

}   // namespace royalbed
