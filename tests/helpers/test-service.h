#pragma once

#include <cstdint>
#include <list>
#include <condition_variable>
#include <memory>
#include <mutex>

#include <corvusoft/restbed/resource.hpp>
#include <corvusoft/restbed/rule.hpp>
#include <corvusoft/restbed/service.hpp>

#include <royalbed/detail/fetch-body-rule.h>

class TestService final
{
public:
    using Resources = std::list<std::shared_ptr<restbed::Resource>>;
    using Rules = std::list<std::shared_ptr<restbed::Rule>>;

    explicit TestService(const Resources& resources,
                         const Rules& rules = {std::make_shared<royalbed::detail::FetchBodyRule>("application/json")});
    ~TestService();

    static constexpr std::uint16_t httpPort = 5555;

private:
    void started();
    void waitForStarted();

    void stopped();
    void waitForStopped();

    std::mutex m_mutex;
    std::condition_variable m_condvar;
    bool m_isRunning = false;
    restbed::Service m_service;
};
