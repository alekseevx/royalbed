#pragma once

#include <exception>
#include <string_view>
#include <type_traits>

#include <corvusoft/restbed/session.hpp>
#include <corvusoft/restbed/status_code.hpp>
#include <nlohmann/json.hpp>
#include "royalbed/detail/traits.h"

namespace royalbed {

void sendText(restbed::Session& session, std::string_view text);
void sendText(restbed::Session& session, int httpStatus, std::string_view text);

void sendJson(restbed::Session& session, const nlohmann::json& value);
void sendJson(restbed::Session& session, int httpStatus, const nlohmann::json& value);

template<typename Value>
void sendJson(restbed::Session& session, int httpStatus, const Value& value)
{
    static_assert(detail::canSerializeJson<Value>, "Value cannot be converted to json."
                                                   "Please define: void to_json(nlohmann::json&, const Value& )"
                                                   "See https://github.com/nlohmann/json#basic-usage");

    sendJson(session, httpStatus, nlohmann::json(value));
}

template<typename Value>
void sendJson(restbed::Session& session, const Value& value)
{
    sendJson<Value>(session, restbed::OK, value);
}

void sendError(restbed::Session& session, std::exception_ptr exPtr);

}   // namespace royalbed
