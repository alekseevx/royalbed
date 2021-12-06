#pragma once

#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace royalbed::server {

enum class ParamLocation
{
    Path,
    Query
};

template<typename T>
struct ParamProperties final
{
    constexpr explicit ParamProperties(std::string_view propName)
      : name(propName)
    {}

    const std::string name;
    ParamLocation loc = ParamLocation::Path;
    bool required = true;
    std::optional<T> defaultValue;
    std::vector<std::function<void(const T&)>> validators;
};

}   // namespace royalbed::server
