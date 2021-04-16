#pragma once

#include <cstdint>
#include <list>
#include <condition_variable>
#include <memory>
#include <mutex>

#include <corvusoft/restbed/resource.hpp>
#include <corvusoft/restbed/service.hpp>

class TestService final
{
public:
    using Resources = std::list<std::shared_ptr<restbed::Resource>>;

    explicit TestService(const Resources& resources);
    ~TestService();

public:
    static constexpr std::uint16_t httpPort = 5555;

private:
    void started();
    void waitForStarted();

    void stopped();
    void waitForStopped();

private:
    std::mutex m_mutex;
    std::condition_variable m_condvar;
    bool m_isRunning = false;
    restbed::Service m_service;
};
