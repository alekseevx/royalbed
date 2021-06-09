#include <chrono>
#include <cstddef>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <utility>

#include <corvusoft/restbed/service.hpp>
#include <corvusoft/restbed/settings.hpp>

#include <nhope/utils/scope-exit.h>

#include <royalbed/detail/fetch-body-rule.h>
#include <royalbed/detail/restbed-logger-impl.h>
#include <royalbed/rest-server.h>
#include <royalbed/router.h>

namespace royalbed {

RestServer::RestServer()
  : m_threadCount(4 * std::thread::hardware_concurrency())
{}

RestServer::~RestServer() = default;

RestServer& RestServer::setLogger(std::shared_ptr<spdlog::logger> logger)
{
    std::scoped_lock lock(m_mutex);

    assert(m_state == State::Stopped);   // NOLINT
    if (m_state != State::Stopped) {
        throw std::runtime_error("Parameters can't be changed for a running HttpServer");
    }

    m_logger = std::move(logger);
    return *this;
}

RestServer& RestServer::setConnectionTimeout(std::chrono::milliseconds timeout)
{
    std::scoped_lock lock(m_mutex);

    assert(m_state == State::Stopped);   // NOLINT
    if (m_state != State::Stopped) {
        throw std::runtime_error("Parameters can't be changed for a running HttpServer");
    }

    m_connectionTimeout = timeout;
    return *this;
}

RestServer& RestServer::setKeepAlive(bool keepAlive)
{
    std::scoped_lock lock(m_mutex);

    assert(m_state == State::Stopped);   // NOLINT
    if (m_state != State::Stopped) {
        throw std::runtime_error("Parameters can't be changed for a running HttpServer");
    }

    m_keepAlive = keepAlive;
    return *this;
}

RestServer& RestServer::setThreadCount(std::size_t threadCount)
{
    std::scoped_lock lock(m_mutex);

    assert(m_state == State::Stopped);   // NOLINT
    if (m_state != State::Stopped) {
        throw std::runtime_error("Parameters can't be changed for a running HttpServer");
    }

    m_threadCount = threadCount;
    return *this;
}

RestServer& RestServer::setBindAddress(std::string_view bindAddress)
{
    std::scoped_lock lock(m_mutex);

    assert(m_state == State::Stopped);   // NOLINT
    if (m_state != State::Stopped) {
        throw std::runtime_error("Parameters can't be changed for a running HttpServer");
    }

    m_bindAddress = bindAddress;
    return *this;
}

RestServer& RestServer::setPort(std::uint16_t port)
{
    std::scoped_lock lock(m_mutex);

    assert(m_state == State::Stopped);   // NOLINT
    if (m_state != State::Stopped) {
        throw std::runtime_error("Parameters can't be changed for a running HttpServer");
    }

    m_port = port;
    return *this;
}

RestServer& RestServer::setReuseAddress(bool reuseAddress)
{
    std::scoped_lock lock(m_mutex);

    assert(m_state == State::Stopped);   // NOLINT
    if (m_state != State::Stopped) {
        throw std::runtime_error("Parameters can't be changed for a running HttpServer");
    }

    m_reuseAddress = reuseAddress;
    return *this;
}

RestServer& RestServer::setRouter(const Router& router)
{
    std::scoped_lock lock(m_mutex);

    assert(m_state == State::Stopped);   // NOLINT
    if (m_state != State::Stopped) {
        throw std::runtime_error("Parameters can't be changed for a running HttpServer");
    }

    m_router = router;
    return *this;
}

void RestServer::listen()
{
    std::shared_ptr<restbed::Settings> engineSettings;
    {
        std::scoped_lock lock(m_mutex);

        assert(m_state == State::Stopped);   // NOLINT
        if (m_state != State::Stopped) {
            throw std::runtime_error("The HttpServer is already running");
        }

        if (m_port == invalidPort) {
            throw std::runtime_error("Pot for incoming connections not specified");
        }

        engineSettings = this->makeRestbedSettings();
        m_engine = this->makeEngine();
        m_state = State::Running;
    }

    nhope::ScopeExit stop([this] {
        std::scoped_lock lock(m_mutex);
        m_engine.reset();
        m_state = State::Stopped;
    });

    m_engine->start(engineSettings);
}

void RestServer::stop()
{
    std::scoped_lock lock(m_mutex);
    if (m_engine != nullptr) {
        m_engine->stop();
    }
}

std::shared_ptr<restbed::Settings> RestServer::makeRestbedSettings() const
{
    auto settings = std::make_shared<restbed::Settings>();
    settings->set_connection_timeout(m_connectionTimeout);
    settings->set_keep_alive(m_keepAlive);
    settings->set_bind_address(m_bindAddress);
    settings->set_port(m_port);
    settings->set_reuse_address(m_reuseAddress);
    settings->set_worker_limit(m_threadCount);
    return settings;
}

[[nodiscard]] std::unique_ptr<restbed::Service> RestServer::makeEngine() const
{
    auto engine = std::make_unique<restbed::Service>();

    if (m_logger != nullptr) {
        const auto logger = std::make_shared<detail::RestbedLoggerImpl>(m_logger);
        engine->set_logger(logger);
    }

    engine->add_rule(std::make_shared<detail::FetchBodyRule>("application/json"));

    for (const auto& resource : m_router.resources()) {
        engine->publish(resource);
    }

    return engine;
}

}   // namespace royalbed
