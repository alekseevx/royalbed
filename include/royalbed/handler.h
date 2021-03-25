#pragma once

#include <exception>
#include <functional>
#include <memory>
#include <utility>
#include <type_traits>

#include <corvusoft/restbed/session.hpp>
#include <corvusoft/restbed/status_code.hpp>

#include <nhope/async/ao-context.h>
#include <nhope/async/future.h>

#include <royalbed/body.h>
#include <royalbed/param.h>
#include <royalbed/send-resp.h>

namespace royalbed {

/**
 * The low-level handler works directly with the session.
 */
using LowLevelHandler = void(std::shared_ptr<restbed::Session>);

/**
 * RequestHandler takes parsed request as arguments (#QueryParam, #PathParam, #Body, etc)
 * The return value is automatically serialized and passed as the response.
 */
template<typename R, typename... Args>
using RequestHandler = R(Args&&...);

/**
 * Similar to RequestHandler, except that the request is processed asynchronously.
 */
template<typename R, typename... Args>
using AsyncRequestHandler = nhope::Future<R>(Args&&...);

template<typename T>
inline constexpr bool isRequstHandlerArg = isParam<T> || isBody<T>;

template<typename Handler>
inline constexpr bool isLowLevelHandler = std::is_invocable_v<Handler, std::shared_ptr<restbed::Session>>;

template<typename... Args>
constexpr void checkRequstHandlerArgTypes()
{
    static_assert((isRequstHandlerArg<Args> && ...), "RequestHandler argument must be one of\n"
                                                     "\tParam <royalbed/param.h>)"
                                                     "\tBody <royalbed/body.h>");
}

template<typename R>
constexpr void checkRequstHandlerResult()
{
    static_assert(std::is_void_v<R> || std::is_constructible_v<nlohmann::json, R>,
                  "The handler result cannot be converted to json."
                  "Please define a to_json function for it."
                  "See https://github.com/nlohmann/json");
}

template<typename R, typename... Args>
std::function<LowLevelHandler> makeLowLevelHandler(int status, std::function<RequestHandler<R, Args...>> handler)
{
    checkRequstHandlerResult<R>();
    checkRequstHandlerArgTypes<Args...>();

    return [status, handler = std::move(handler)](std::shared_ptr<restbed::Session> session) {
        try {
            if constexpr (std::is_void_v<R>) {
                handler(Args(*session)...);
                session->close(status);
            } else {
                R result = handler(Args(*session)...);
                sendJson(*session, status, result);
            }
        } catch (...) {
            auto exPtr = std::current_exception();
            sendError(*session, std::move(exPtr));
        }
    };
}

// template<typename R, typename... Args>
// std::function<LowLevelHandler> makeLowLevelHandler(std::function<nhope::Future<R>(Args&&...)>&& handler)
// {
//     checkRequstHandlerArgTypes<Args...>();
// }

template<typename Handler>
std::function<LowLevelHandler> makeLowLevelHandler(int status, Handler&& handler)
{
    return makeLowLevelHandler(status, std::function(handler));
}

}   // namespace royalbed
