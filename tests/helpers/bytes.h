#pragma once

#include <cstdint>
#include <span>
#include <string>

inline std::string asString(std::span<const std::uint8_t> bytes)
{
    return {
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
      reinterpret_cast<const char*>(bytes.data()),
      bytes.size(),
    };
}
