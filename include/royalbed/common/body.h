#pragma once

#include <exception>
#include <memory>
#include <string_view>
#include <type_traits>

#include <fmt/core.h>
#include <nlohmann/json.hpp>

#include "nhope/async/future.h"
#include "nhope/io/io-device.h"

#include "royalbed/common/detail/traits.h"
#include "royalbed/common/http-error.h"
#include "royalbed/common/http-status.h"
#include "royalbed/common/headers.h"

namespace royalbed::common {

enum class BodyType
{
    Json,
    Xml,
    Plain
};
BodyType extractBodyType(const Headers& headers);

template<typename T, BodyType B = BodyType::Json>
class Body final
{
public:
    using Type = T;

    Body(const Body<T>& b)
      : m_data(b.m_data)
    {}

    Body(T val)
      : m_data(std::move(val))
    {}

    static BodyType constexpr type()
    {
        return B;
    }

    const T& operator*() const
    {
        return m_data;
    }

    const T* operator->() const
    {
        return &m_data;
    }

    [[nodiscard]] const T& get() const
    {
        return m_data;
    }

private:
    T m_data;
};

template<typename T>
static constexpr bool isBody = false;
template<typename T>
static constexpr bool isBody<Body<T, BodyType::Json>> = true;
template<typename T>
static constexpr bool isBody<Body<T, BodyType::Plain>> = true;
template<typename T>
static constexpr bool isBody<Body<T, BodyType::Xml>> = true;

nlohmann::json getJson(const std::vector<std::uint8_t>& bodyData);

template<typename T>
Body<T> parseBody(const Headers& /*req*/, const std::vector<std::uint8_t>& rawBody)
{
    if constexpr (!detail::canDeserializeJson<T>) {
        static_assert(!std::is_same_v<T, T>, "T cannot be retrived from json."
                                             "need implement: void from_json(const nlohmann::json&, T& )"
                                             "See https://github.com/nlohmann/json#basic-usage");
    }

    try {
        // TODO parse content
        // const BodyType bodyType = extractBodyType(req);

        const auto jsonValue = getJson(rawBody);
        return jsonValue.get<T>();

    } catch (const std::exception& ex) {
        const auto message = fmt::format("Failed to parse request body: {}", ex.what());
        throw HttpError(HttpStatus::BadRequest, message);
    }
}

template<typename T>
struct IsBodyType
{
    static constexpr bool value = common::isBody<T>;
};

}   // namespace royalbed::common

template<typename T>
concept BodyTypename = royalbed::common::isBody<T>;
