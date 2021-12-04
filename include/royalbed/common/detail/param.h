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
#include "royalbed/common/request.h"
#include "royalbed/common/detail/param-setters.h"

namespace royalbed::common::detail {

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
concept Conflictable = requires
{
    {
        T::conflicts()

        } -> std::same_as<std::tuple<>>;
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
void checkProps()
{
    //TODO make it
    if constexpr (Conflictable<T>) {
        int x = T::conflicts();
    }
}

template<typename T, ParametrSetterable<T>... Setters>
ParamProperties<T> makeProperties(std::string_view name)
{
    checkProps<Setters...>();

    ParamProperties<T> props(name);
    initProperties<T, Setters...>(props);
    return props;
}

template<typename T>
std::optional<T> getParam(const common::Request& /*req*/, const ParamProperties<T>& /*paramProps*/);

template<typename T, StringLiteral name, ParametrSetterable<T>... Properties>
class Param final
{
public:
    Param(Param&&) noexcept = default;
    Param(const Param&) = default;
    Param& operator=(const Param&) = default;
    Param& operator=(Param&&) noexcept = default;

    explicit Param(const royalbed::common::Request& req)
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

template<typename T, StringLiteral name, ParametrSetterable<T>... Properties>
using PathParam = Param<T, name, ParamLocProp<ParamLocation::Path>, Properties...>;

template<typename T, StringLiteral name, ParametrSetterable<T>... Properties>
using QueryParam = Param<T, name, ParamLocProp<ParamLocation::Query>, Properties...>;

std::optional<std::string> getParam(const common::Request& req, const std::string& name, ParamLocation loc,
                                    bool required);

template<typename T>
std::optional<T> getParam(const common::Request& req, const ParamProperties<T>& paramProps)
{
    const std::optional<std::string> param = getParam(req, paramProps.name, paramProps.loc, paramProps.required);
    try {
        if (!param.has_value()) {
            return paramProps.defaultValue;
        }

        T val = fromString<T>(param.value());
        if (paramProps.validate != nullptr) {
            paramProps.validate(val);
        }
        return val;

    } catch (const std::exception& ex) {
        const auto message = fmt::format("Failed to get '{}' parameter: {}", paramProps.name, ex.what());
        throw HttpError(HttpStatus::BadRequest, message);
    }
}

}   // namespace royalbed::common::detail
