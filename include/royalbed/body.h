#pragma once

#include <memory>
#include <type_traits>

#include <corvusoft/restbed/request.hpp>
#include <corvusoft/restbed/session.hpp>
#include <fmt/core.h>
#include <nlohmann/json.hpp>

#include "royalbed/detail/traits.h"
#include "royalbed/http-error.h"
#include "royalbed/http-status.h"

namespace royalbed {

template<typename T>
T parseBody(const restbed::Request& request);

template<typename T>
T parseBody(const restbed::Session& session);

template<typename T>
class Body final
{
public:
    Body() = default;
    Body(Body&&) noexcept = default;
    Body(const Body&) = default;

    Body& operator=(const Body&) = default;
    Body& operator=(Body&&) noexcept = default;

    explicit Body(const restbed::Session& session)
      : m_data(parseBody<T>(session))
    {}

    explicit Body(const restbed::Request& request)
      : m_data(parseBody<T>(request))
    {}

    const T& operator*() const
    {
        return m_data;
    }

    const T* operator->() const
    {
        return &m_data;
    }

    const T& get() const
    {
        return m_data;
    }

private:
    T m_data;
};

template<typename T>
inline constexpr bool isBody = false;

template<typename T>
inline constexpr bool isBody<Body<T>> = true;

template<typename T>
T parseBody(const restbed::Request& req)
{
    if constexpr (!detail::canDeserializeJson<T>) {
        static_assert(!std::is_same_v<T, T>, "T cannot be retrived from json."
                                             "need implement: void from_json(const nlohmann::json&, T& )"
                                             "See https://github.com/nlohmann/json#basic-usage");
    }

    const auto jsonValue = parseBody<nlohmann::json>(req);

    try {
        return jsonValue.get<T>();
    } catch (const std::exception& ex) {
        const auto message = fmt::format("Failed to parse request body: {}", ex.what());
        throw HttpError(HttpStatus::BadRequest, message);
    }
}

template<typename T>
T parseBody(const restbed::Session& session)
{
    return parseBody<T>(*session.get_request());
}

template<>
nlohmann::json parseBody<nlohmann::json>(const restbed::Request& req);

}   // namespace royalbed
