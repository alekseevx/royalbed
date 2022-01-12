#include <memory>
#include "spdlog/logger.h"
#include "spdlog/sinks/null_sink.h"

inline std::shared_ptr<spdlog::logger> nullLogger()
{
    static auto log = spdlog::null_logger_mt("TestLogger");
    return log;
}
