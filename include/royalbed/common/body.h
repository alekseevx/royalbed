#pragma once

#include <exception>
#include <memory>
#include <type_traits>

#include <fmt/core.h>
#include <nlohmann/json.hpp>

#include "nhope/async/future.h"
#include "nhope/io/io-device.h"

#include "royalbed/common/detail/traits.h"
#include "royalbed/common/http-error.h"
#include "royalbed/common/http-status.h"
#include "royalbed/common/request.h"

namespace royalbed::common {

enum class BodyType
{
    Json,
    Xml,
    Plain,
    Stream,
    NoBody
};

template<typename T>
class Body final
{
public:
    Body(const Body<T>& b)
      : m_data(b.m_data)
    {}

    Body(T val)
      : m_data(val)
    {}

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

struct NoneBody
{};

BodyType extractBodyType(Request& req);

template<typename T>
static constexpr bool isBody = false;
template<typename T>
static constexpr bool isBody<Body<T>> = true;

nlohmann::json getJson(const std::vector<std::uint8_t>& bodyData);

template<typename T>
Body<T> parseBody(Request& req, const std::vector<std::uint8_t>& rawBody)
{
    if constexpr (!detail::canDeserializeJson<T>) {
        static_assert(!std::is_same_v<T, T>, "T cannot be retrived from json."
                                             "need implement: void from_json(const nlohmann::json&, T& )"
                                             "See https://github.com/nlohmann/json#basic-usage");
    }

    // auto bodyType = extractBodyType(req);
    // if (bodyType == BodyType::NoBody) {
    //     return Body<NoneBody>{};
    // }

    try {
        const auto jsonValue = getJson(rawBody);
        return jsonValue.get<T>();
    } catch (const std::exception& ex) {
        const auto message = fmt::format("Failed to parse request body: {}", ex.what());
        throw HttpError(HttpStatus::BadRequest, message);
    }
}

}   // namespace royalbed::common

template<typename T>
concept BodyTypename = royalbed::common::isBody<T>;
