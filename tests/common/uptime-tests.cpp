#include <chrono>
#include <thread>

#include <gtest/gtest.h>
#include "spdlog/sinks/null_sink.h"

#include <royalbed/common/detail/uptime.h>

using namespace std::literals;

TEST(UpTime, Expired)   // NOLINT
{
    royalbed::common::detail::UpTimeLogger log(spdlog::null_logger_st("test"));
    const std::chrono::nanoseconds expired{100ms};
    std::this_thread::sleep_for(expired);
    EXPECT_GE(log.expired(), expired);
}
