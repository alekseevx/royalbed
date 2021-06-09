#pragma once

#include <memory>

#include <spdlog/logger.h>
#include <corvusoft/restbed/logger.hpp>

namespace royalbed::detail {

class RestbedLoggerImpl final : public restbed::Logger
{
public:
    explicit RestbedLoggerImpl(std::shared_ptr<spdlog::logger> logger);
    ~RestbedLoggerImpl() override;

    void stop() override;
    void start(const std::shared_ptr<const restbed::Settings>& settings) override;
    void log(Level level, const char* format, ...) override;
    void log_if(bool expression, Level level, const char* format, ...) override;

private:
    std::shared_ptr<spdlog::logger> m_logger;
};

}   // namespace royalbed::detail
