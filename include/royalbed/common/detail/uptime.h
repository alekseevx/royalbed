#pragma once

#include <chrono>
#include <string_view>

#include "spdlog/logger.h"
#include "nhope/utils/detail/fast-pimpl.h"

namespace royalbed::common::detail {

class UpTimeLogger final
{
public:
    explicit UpTimeLogger(std::shared_ptr<spdlog::logger> logger, std::string_view lastWord = "uptime");
    ~UpTimeLogger();

    [[nodiscard("must check expired time")]] std::chrono::nanoseconds expired() const;

private:
    class Impl;
    static constexpr std::size_t implSize{128};
    nhope::detail::FastPimpl<Impl, implSize> m_impl;
};

}   // namespace royalbed::common::detail
