#pragma once

#include <memory>

#include <corvusoft/restbed/request.hpp>
#include <corvusoft/restbed/status_code.hpp>
#include <fmt/core.h>
#include <nlohmann/json.hpp>

#include <royalbed/http-error.h>

namespace royalbed {

template<typename T>
T parseBody(const restbed::Request& request);

template<typename T>
class Body final
{
public:
    Body() = default;
    Body(Body&&) noexcept = default;
    Body(const Body&) = default;

    Body& operator=(const Body&) = default;
    Body& operator=(Body&&) noexcept = default;

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

    const T& get()
    {
        return m_data;
    }

private:
    T m_data;
};

template<typename T>
T parseBody(const restbed::Request& req)
{
    const auto jsonValue = parseBody<nlohmann::json>(req);

    try {
        return jsonValue.get<T>();
    } catch (const std::exception& ex) {
        const auto message = fmt::format("Failed to parse request body: {}", ex.what());
        throw HttpError(restbed::BAD_REQUEST, message);
    }
}

template<>
nlohmann::json parseBody<nlohmann::json>(const restbed::Request& req);

}   // namespace royalbed
