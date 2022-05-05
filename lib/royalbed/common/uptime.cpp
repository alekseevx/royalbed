#include <algorithm>
#include <string_view>

#include "fmt/chrono.h"

#include "royalbed/common/detail/uptime.h"

namespace royalbed::common::detail {

using namespace std::chrono;
class UpTimeLogger::Impl
{
public:
    explicit Impl(std::shared_ptr<spdlog::logger>&& logger, std::string_view lastWord)
      : m_log(std::move(logger))
      , m_lastWord(lastWord)
    {}

    ~Impl()
    {
        const auto upTime = expired();
        if (upTime > days(1)) {
            m_log->debug("{0} {1:}", m_lastWord, duration_cast<hours>(upTime));
        } else {
            m_log->debug("{0} {1:%H:%M:%S}", m_lastWord, upTime);
        }
    }

    [[nodiscard]] std::chrono::nanoseconds expired() const noexcept
    {
        return steady_clock::now() - m_startTime;
    }

private:
    steady_clock::time_point m_startTime{steady_clock::now()};
    std::shared_ptr<spdlog::logger> m_log;
    const std::string m_lastWord;
};

std::chrono::nanoseconds UpTimeLogger::expired() const
{
    return m_impl->expired();
}

UpTimeLogger::UpTimeLogger(std::shared_ptr<spdlog::logger> logger, std::string_view lastWord)
  : m_impl(std::move(logger), lastWord)
{}

UpTimeLogger::~UpTimeLogger() = default;

}   // namespace royalbed::common::detail
