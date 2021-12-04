#pragma once

#include <algorithm>
#include <cstddef>
#include <concepts>
#include <string_view>
#include <functional>
#include <optional>
#include <type_traits>
#include <utility>

#include "royalbed/common/request.h"

namespace royalbed::common::detail {

template<size_t N>
struct StringLiteral
{
    constexpr StringLiteral(const char (&str)[N])
      : val(str)
    {}
    const std::string_view val;
};

enum class ParamLocation
{
    Path,
    Query
};

template<typename T>
struct ParamProperties final
{
    constexpr ParamProperties(std::string_view propName)
      : name(propName)
    {}
    const std::string_view name;
    ParamLocation loc = ParamLocation::Path;
    bool required = true;
    std::optional<T> defaultValue;
    std::function<void(const T&)> validate;
};

template<typename T, typename P>
concept ParametrSetterable = requires
{
    {
        T::set(std::declval<ParamProperties<P>&>())
        } -> std::same_as<void>;
};

template<typename T>
void initProperties(ParamProperties<T>& properties)
{}

template<typename T, ParametrSetterable<T> HeadProperty, ParametrSetterable<T>... TailProperties>
void initProperties(ParamProperties<T>& properties)
{
    HeadProperty::set(properties);
    initProperties<T, TailProperties...>(properties);
}

template<typename T, ParametrSetterable<T>... Setters>
ParamProperties<T> makeProperties(std::string_view name)
{
    //TODO check props
    ParamProperties<T> props(name);
    initProperties<T, Setters...>(props);
    return props;
}

template<typename T, StringLiteral name, ParametrSetterable<T>... Properties>
class Param final
{
public:
    Param(Param&&) noexcept = default;
    Param(const Param&) = default;
    Param& operator=(const Param&) = default;
    Param& operator=(Param&&) noexcept = default;

    // explicit Param(const royalbed::common::Request& req)
    //   : m_data(getParam<T>(req, Param::props()))
    // {
    //     // req.uri.path;
    // }

    // explicit Param(const restbed::Session& session)
    //   : m_data(getParam<T>(session, Param::props()))
    // {}

    const T& operator*() const
    {
        return m_data.value();
    }

    const T* operator->() const
    {
        return &m_data;
    }

    [[nodiscard]] bool hasValue() const
    {
        return m_data.has_value();
    }

    [[nodiscard]] const T& get() const
    {
        return m_data.value();
    }

    operator const std::optional<T>&() const
    {
        return m_data;
    }

    static const ParamProperties<T>& props()
    {
        static const auto props = makeProperties<T, Properties...>(name);
        return props;
    }

private:
    std::optional<T> m_data;
};

}   // namespace royalbed::common::detail
