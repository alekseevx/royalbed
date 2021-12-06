#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <tuple>
#include <string_view>

#include <fmt/format.h>

#include "royalbed/server/string-literal.h"
#include "royalbed/server/param-properties.h"
#include "royalbed/server/http-error.h"
#include "royalbed/server/http-status.h"

namespace royalbed::server {

struct Required final
{
    template<typename T>
    static void set(ParamProperties<T>& properties)
    {
        properties.required = true;
    }
};

struct NotRequired final
{
    template<typename T>
    static void set(ParamProperties<T>& properties)
    {
        properties.required = false;
    }

    static constexpr std::tuple<Required> confilcts{{}};
};

template<const auto& value>
struct DefaultValue final
{
    template<typename T>
    static void set(ParamProperties<T>& properties)
    {
        properties.defaultValue = T(value);
    }
};

template<std::int64_t T>
struct DefaultInt final
{
    template<typename V>
    static void set(ParamProperties<V>& properties)
    {
        properties.defaultValue.emplace(T);
    }
};

template<StringLiteral value>
struct DefaultStr final
{
    template<typename V>
    static void set(ParamProperties<V>& properties)
    {
        properties.defaultValue.emplace(value);
    }
};

template<std::int64_t MaxVal>
struct Max final
{
    template<typename T>
    static void validate(ParamProperties<T>& properties)
    {
        properties.validators.emplace_back([](const auto& val) {
            if (val > MaxVal) {
                throw common::HttpError(common::HttpStatus::BadRequest, fmt::format("param value to big: {}", val));
            }
        });
    }
};

template<std::int64_t MinVal>
struct Min final
{
    template<typename T>
    static void validate(ParamProperties<T>& properties)
    {
        properties.validators.emplace_back([](const auto& val) {
            if (val < MinVal) {
                throw common::HttpError(common::HttpStatus::BadRequest, fmt::format("param value to small: {}", val));
            }
        });
    }
};

}   // namespace royalbed::server
