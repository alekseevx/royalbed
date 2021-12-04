#pragma once

#include <tuple>

namespace royalbed::common::detail {

template<typename T>
struct ParamProperties;

struct Required final
{
    template<typename T>
    static void set(ParamProperties<T>& properies)
    {
        properies.required = true;
    }
};

struct NotRequired final
{
    template<typename T>
    static void set(ParamProperties<T>& properies)
    {
        properies.required = false;
    }

    static constexpr std::tuple<Required> confilcts{{}};
};

template<const auto& value>
struct DefaultValue final
{
    template<typename T>
    static void set(ParamProperties<T>& properies)
    {
        properies.defaultValue = T(value);
    }
};

}   // namespace royalbed::common::detail
