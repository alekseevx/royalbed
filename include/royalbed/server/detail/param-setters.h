#pragma once

#include <algorithm>
#include <cstddef>
#include <tuple>
#include <string_view>

namespace royalbed::server::detail {

template<size_t N>
struct StringLiteral
{
    constexpr StringLiteral(const char (&str)[N])
    {
        std::copy_n(str, N, val);
    }
    char val[N];
};

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

template<int T>
struct DefaultInt final
{
    template<typename V>
    static void set(ParamProperties<V>& properies)
    {
        properies.defaultValue.emplace(T);
    }
};

template<StringLiteral T>
struct DefaultStr final
{
    template<typename V>
    static void set(ParamProperties<V>& properies)
    {
        properies.defaultValue.emplace(T.val);
    }
};

}   // namespace royalbed::server::detail
