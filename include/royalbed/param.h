#pragma once

#include <array>
#include <cstddef>
#include <exception>
#include <functional>
#include <optional>
#include <string>
#include <string_view>

#include <corvusoft/restbed/request.hpp>
#include <corvusoft/restbed/status_code.hpp>
#include <fmt/core.h>

#include <royalbed/http-error.h>
#include <royalbed/string.h>

namespace royalbed {

enum class ParamLoc
{
    Path,
    Query
};

template<typename T>
struct ParamProperties;

// Param Properties
struct NotRequired;
struct Required;
template<const auto& value>
struct DefaultValue;

template<ParamLoc loc>
struct ParamLocProp;

template<typename T>
std::optional<T> getParam(const restbed::Request& req, const ParamProperties<T>& paramProps);

template<typename T, typename... Properties>
ParamProperties<T> makeProperties(std::string_view name);

template<typename T, const std::string_view& name, typename... Properties>
class Param final
{
public:
    Param() = default;
    Param(Param&&) noexcept = default;
    Param(const Param&) = default;

    Param& operator=(const Param&) = default;
    Param& operator=(Param&&) noexcept = default;

    explicit Param(const restbed::Request& req)
      : m_data(getParam<T>(req, Param::props()))
    {}

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

public:
    static const ParamProperties<T>& props()
    {
        static const auto props = makeProperties<T, Properties...>(name);
        return props;
    }

private:
    std::optional<T> m_data;
};

template<typename T, const std::string_view& name, typename... Properties>
using PathParam = Param<T, name, ParamLocProp<ParamLoc::Path>, Properties...>;

template<typename T, const std::string_view& name, typename... Properties>
using QueryParam = Param<T, name, ParamLocProp<ParamLoc::Query>, Properties...>;

template<typename T>
struct ParamProperties final
{
    std::string name;

    ParamLoc loc = ParamLoc::Path;
    bool required = true;
    std::optional<T> defaultValue;
    std::function<void(const T&)> checker;
};

template<typename T>
void initProperties(ParamProperties<T>& properties)
{}

template<typename T, typename HeadProperty, typename... TailProperties>
void initProperties(ParamProperties<T>& properties)
{
    HeadProperty::set(properties);
    return initProperties<T, TailProperties...>(properties);
}

template<typename T, typename... Properties>
ParamProperties<T> makeProperties(std::string_view name)
{
    ParamProperties<T> properties{std::string(name)};
    initProperties<T, Properties...>(properties);
    return properties;
}

template<ParamLoc loc>
struct ParamLocProp final
{
    template<typename T>
    static void set(ParamProperties<T>& properies)
    {
        properies.loc = loc;
    }
};

struct NotRequired final
{
    template<typename T>
    static void set(ParamProperties<T>& properies)
    {
        properies.required = false;
    }
};

struct Required final
{
    template<typename T>
    static void set(ParamProperties<T>& properies)
    {
        properies.required = true;
    }
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

std::optional<std::string> getParam(const restbed::Request& req, const std::string& name, ParamLoc loc, bool required);

template<typename T>
std::optional<T> getParam(const restbed::Request& req, const ParamProperties<T>& paramProps)
{
    const std::optional<std::string> param = getParam(req, paramProps.name, paramProps.loc, paramProps.required);
    try {
        if (!param.has_value()) {
            return paramProps.defaultValue;
        }

        T val = fromString<T>(param.value());
        if (paramProps.checker != nullptr) {
            paramProps.checker(val);
        }
        return val;

    } catch (const std::exception& ex) {
        const auto message = fmt::format("Failed to get '{}' parameter: {}", paramProps.name, ex.what());
        throw HttpError(restbed::BAD_REQUEST, message);
    }
}

}   // namespace royalbed
