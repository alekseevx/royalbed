#include <cstdarg>
#include <cstdio>
#include <utility>
#include <memory>

#include <spdlog/logger.h>
#include <corvusoft/restbed/logger.hpp>

#include <royalbed/detail/restbed-logger-impl.h>

namespace royalbed::detail {

namespace {

spdlog::level::level_enum toSpdLevel(restbed::Logger::Level level)
{
    switch (level) {
    case restbed::Logger::Level::INFO:
        return spdlog::level::info;

    case restbed::Logger::Level::DEBUG:
        return spdlog::level::debug;

    case restbed::Logger::Level::FATAL:
        return spdlog::level::critical;

    case restbed::Logger::Level::ERROR:
        return spdlog::level::err;

    case restbed::Logger::Level::WARNING:
        return spdlog::level::warn;

    case restbed::Logger::Level::SECURITY:
        return spdlog::level::critical;
    default:
        return spdlog::level::off;
    };
}

void vlog(spdlog::logger& logger, restbed::Logger::Level level, const char* format, std::va_list args)
{
    auto spdLevel = toSpdLevel(level);
    if (!logger.should_log(spdLevel)) {
        return;
    }

    constexpr auto bufSize = 256;
    char buf[bufSize];                            // NOLINT
    std::vsnprintf(buf, bufSize, format, args);   // NOLINT
    logger.log(spdLevel, buf);
}

}   // namespace

RestbedLoggerImpl::RestbedLoggerImpl(std::shared_ptr<spdlog::logger> logger)
  : m_logger(std::move(logger))
{}

RestbedLoggerImpl::~RestbedLoggerImpl() = default;

void RestbedLoggerImpl::stop()
{}

void RestbedLoggerImpl::start(const std::shared_ptr<const restbed::Settings>& /*unused*/)
{}

// NOLINTNEXTLINE
void RestbedLoggerImpl::log(const Level level, const char* format, ...)
{
    std::va_list args;                      // NOLINT
    va_start(args, format);                 // NOLINT
    vlog(*m_logger, level, format, args);   // NOLINT
    va_end(args);                           // NOLINT
}

// NOLINTNEXTLINE
void RestbedLoggerImpl::log_if(bool expression, const Level level, const char* format, ...)
{
    if (!expression) {
        return;
    }

    std::va_list args;                      // NOLINT
    va_start(args, format);                 // NOLINT
    vlog(*m_logger, level, format, args);   // NOLINT
    va_end(args);                           // NOLINT
}

}   // namespace royalbed::detail
