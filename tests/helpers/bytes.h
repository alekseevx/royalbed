#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

inline std::string asString(std::span<const std::uint8_t> bytes)
{
    return {
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
      reinterpret_cast<const char*>(bytes.data()),
      bytes.size(),
    };
}

inline std::vector<std::uint8_t> asBytes(std::string_view str)
{
    return {
      // NOLINTNEXTLINE
      reinterpret_cast<const std::uint8_t*>(str.data()),
      // NOLINTNEXTLINE
      reinterpret_cast<const std::uint8_t*>(str.data()) + str.size(),
    };
}
