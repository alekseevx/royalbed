#pragma once

#include <utility>

#include "royalbed/server/detail/extract-param.h"
#include "royalbed/server/param-properties.h"
#include "royalbed/server/param-setters.h"
#include "royalbed/server/string-literal.h"

namespace royalbed::server {

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
    // checkProps<Setters...>();

    ParamProperties<T> props(name);
    initProperties<T, Setters...>(props);
    return props;
}

template<typename T, StringLiteral name, ParametrSettings<T>... Properties>
class Param final
{
public:
    Param(Param&&) noexcept = default;
    Param(const Param&) = default;
    Param& operator=(const Param&) = default;
    Param& operator=(Param&&) noexcept = default;

    Param(const RequestContext& req)
      : m_data(detail::extractParam<T>(req, Param::props()))
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

    static const ParamProperties<T>& props()
    {
        static const auto props = makeProperties<T, Properties...>(name);
        return props;
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

template<typename T>
static constexpr bool isQueryOrParam = false;
template<typename T, StringLiteral name, ParametrSettings<T>... Properties>
static constexpr bool isQueryOrParam<Param<T, name, Properties...>> = true;

}   // namespace royalbed::server
