#pragma once

#include <array>
#include <cstddef>
#include <string_view>

namespace royalbed::server {

template<std::size_t N>
struct StringLiteral
{
    constexpr StringLiteral(const char (&str)[N])
      : data(std::to_array(str))
    {}

    constexpr operator std::string_view() const
    {
        return {data.data()};
    }

    const std::array<char, N> data;
};

}   // namespace royalbed::server
