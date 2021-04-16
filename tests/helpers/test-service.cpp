#include <cassert>
#include <chrono>
#include <memory>
#include <mutex>
#include <thread>

#include <royalbed/rules/fetch-body-rule.h>
#include <corvusoft/restbed/settings.hpp>

#include "test-service.h"

using namespace std::literals;

TestService::TestService(const Resources& resources)
{
    m_service.add_rule(std::make_shared<royalbed::FetchBodyRule>("application/json"));

    for (const auto& resource : resources) {
        m_service.publish(resource);
    }

    std::thread([this, resources] {
        auto settings = std::make_shared<restbed::Settings>();
        settings->set_port(httpPort);
        settings->set_reuse_address(true);
        settings->set_bind_address("127.0.0.1");

        this->started();
        m_service.start(settings);
        this->stopped();
    }).detach();

    this->waitForStarted();
}

TestService::~TestService()
{
    m_service.stop();
    this->waitForStopped();
}

void TestService::started()
{
    std::scoped_lock lock(m_mutex);
    m_isRunning = true;
    m_condvar.notify_all();
}

void TestService::waitForStarted()
{
    std::unique_lock lock(m_mutex);
    m_condvar.wait(lock, [this] {
        return m_isRunning;
    });

    // FIXME: Wait for call m_service.start(settings) and open port;
    std::this_thread::sleep_for(100ms);
}

void TestService::stopped()
{
    std::scoped_lock lock(m_mutex);
    m_isRunning = false;
    m_condvar.notify_all();
}

void TestService::waitForStopped()
{
    std::unique_lock lock(m_mutex);
    m_condvar.wait(lock, [this] {
        return !m_isRunning;
    });
}
