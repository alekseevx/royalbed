#pragma once

#include <exception>
#include <memory>
#include <type_traits>

#include <fmt/core.h>
#include <nlohmann/json.hpp>

#include "nhope/async/future.h"
#include "royalbed/common/detail/traits.h"
#include "royalbed/common/http-error.h"
#include "royalbed/common/http-status.h"
#include "royalbed/server/request-context.h"
#include "royalbed/server/request.h"

namespace royalbed::common {

enum class BodyType
{
    Json,
    Xml,
    Plain,
    Stream
};

template<typename T>
nhope::Future<T> parseBody(server::RequestContext& req);

template<typename T, BodyType B = BodyType::Json>
class Body final
{
public:
    nhope::Future<T> get(server::RequestContext& req)
    {
        checkMediaType(req.request);
        return parseBody<T>(req);
    }

private:
    void checkMediaType(const server::Request& r)
    {
        using namespace std::literals;

        const auto contentType = r.headers.at("Content-Type");
        if constexpr (B == BodyType::Json) {
            if (contentType != "application/json"sv) {
                throw HttpError(HttpStatus::UnsupportedMediaType, "Request body must be JSON");
            }
        }
    }
};

template<typename T>
inline constexpr bool isBody = false;
template<typename T, BodyType B>
inline constexpr bool isBody<Body<T, B>> = true;

template<typename T>
inline constexpr bool isJsonBody = false;
template<typename T>
inline constexpr bool isJsonBody<Body<T, BodyType::Json>> = true;

template<typename T>
inline constexpr bool isXmlBody = false;
template<typename T>
inline constexpr bool isXmlBody<Body<T, BodyType::Xml>> = true;

template<typename T>
inline constexpr bool isPlainBody = false;
template<typename T>
inline constexpr bool isPlainBody<Body<T, BodyType::Plain>> = true;

template<typename T>
inline constexpr bool isStreamBody = false;
template<typename T>
inline constexpr bool isStreamBody<Body<T, BodyType::Stream>> = true;

template<typename T>
nhope::Future<T> parseBody(server::RequestContext& req)
{
    if constexpr (!detail::canDeserializeJson<T>) {
        static_assert(!std::is_same_v<T, T>, "T cannot be retrived from json."
                                             "need implement: void from_json(const nlohmann::json&, T& )"
                                             "See https://github.com/nlohmann/json#basic-usage");
    }

    return parseBody<nlohmann::json>(req)
      .then(req.aoCtx,
            [](nlohmann::json jsonValue) {
                return jsonValue.get<T>();
            })
      .fail(req.aoCtx, [](auto err) -> T {
          try {
              std::rethrow_exception(std::move(err));
          } catch (const std::exception& ex) {
              const auto message = fmt::format("Failed to parse request body: {}", ex.what());
              throw HttpError(HttpStatus::BadRequest, message);
          }
      });
}

template<>
nhope::Future<nlohmann::json> parseBody<nlohmann::json>(server::RequestContext& req);

}   // namespace royalbed::common
