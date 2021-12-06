#pragma once

#include <algorithm>
#include <cstddef>
#include <concepts>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

#include "fmt/format.h"
#include "nhope/utils/type.h"

#include "royalbed/common/http-error.h"
#include "royalbed/common/http-status.h"
#include "royalbed/common/detail/string-utils.h"
#include "royalbed/server/request-context.h"
#include "royalbed/server/detail/param-setters.h"

namespace royalbed::server::detail {

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
    const std::string name;
    ParamLocation loc = ParamLocation::Path;
    bool required = true;
    std::optional<T> defaultValue;
    std::vector<std::function<void(const T&)>> validators;
};

template<typename T, typename P>
concept ParametrSetterable = requires
{
    {
        T::set(std::declval<ParamProperties<P>&>())
        } -> std::same_as<void>;
};

template<typename T, typename P>
concept ParametrValidateble = requires
{
    {
        T::validate(std::declval<ParamProperties<P>&>())
        } -> std::same_as<void>;
};

template<typename T, typename P>
concept ParametrSettings = ParametrValidateble<T, P> || ParametrSetterable<T, P>;

template<typename T>
concept Conflictable = requires
{
    {
        T::conflicts()

        } -> std::same_as<std::tuple<>>;
};

template<typename T>
void initProperties(ParamProperties<T>& properties)
{}

template<typename T, ParametrSettings<T> HeadProperty, ParametrSettings<T>... TailProperties>
void initProperties(ParamProperties<T>& properties)
{
    if constexpr (ParametrSetterable<HeadProperty, T>) {
        HeadProperty::set(properties);
    } else if constexpr (ParametrValidateble<HeadProperty, T>) {
        HeadProperty::validate(properties);
    }
    initProperties<T, TailProperties...>(properties);
}

template<typename T, ParametrSettings<T>... Setters>
void checkProps()
{
    //TODO make it
    if constexpr (Conflictable<T>) {
        auto v = T::conflicts();
    }
}

template<typename T, ParametrSettings<T>... Setters>
ParamProperties<T> makeProperties(std::string_view name)
{
    checkProps<Setters...>();

    ParamProperties<T> props(name);
    initProperties<T, Setters...>(props);
    return props;
}

template<typename T>
std::optional<T> getParam(const server::RequestContext& /*req*/, const ParamProperties<T>& /*paramProps*/);

template<typename T, StringLiteral name, ParametrSettings<T>... Properties>
class Param final
{
public:
    Param(Param&&) noexcept = default;
    Param(const Param&) = default;
    Param& operator=(const Param&) = default;
    Param& operator=(Param&&) noexcept = default;

    explicit Param(const server::RequestContext& req)
      : m_data(getParam<T>(req, makeProperties<T, Properties...>(name.val)))
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

private:
    std::optional<T> m_data;
};

template<ParamLocation loc>
struct ParamLocProp final
{
    template<typename T>
    static void set(ParamProperties<T>& properies)
    {
        properies.loc = loc;
    }
};

template<typename T, StringLiteral name, ParametrSettings<T>... Properties>
using PathParam = Param<T, name, ParamLocProp<ParamLocation::Path>, Properties...>;

template<typename T, StringLiteral name, ParametrSettings<T>... Properties>
using QueryParam = Param<T, name, ParamLocProp<ParamLocation::Query>, Properties...>;

std::optional<std::string> getParam(const server::RequestContext& req, const std::string& name, ParamLocation loc,
                                    bool required);

template<typename T>
std::optional<T> getParam(const server::RequestContext& req, const ParamProperties<T>& paramProps)
{
    const std::optional<std::string> param = getParam(req, paramProps.name, paramProps.loc, paramProps.required);
    try {
        if (!param.has_value()) {
            return paramProps.defaultValue;
        }

        T val = common::detail::fromString<T>(param.value());
        for (const auto& validate : paramProps.validators) {
            validate(val);
        }
        return val;

    } catch (const std::exception& ex) {
        const auto message = fmt::format("Failed to get '{}' parameter: {}", paramProps.name, ex.what());
        throw common::HttpError(common::HttpStatus::BadRequest, message);
    }
}

}   // namespace royalbed::server::detail
