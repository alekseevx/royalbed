#pragma once

#include "royalbed/detail/traits.h"
#include <exception>
#include <functional>
#include <memory>
#include <utility>
#include <type_traits>

#include <corvusoft/restbed/session.hpp>
#include <corvusoft/restbed/status_code.hpp>

#include <nhope/async/ao-context.h>
#include <nhope/async/future.h>
#include <nhope/utils/type.h>

#include <royalbed/body.h>
#include <royalbed/param.h>
#include <royalbed/send-resp.h>

namespace royalbed {

/**
 * A low-level handler works directly with the session.
 */
using LowLevelHandler = void(std::shared_ptr<restbed::Session>);

/**
 * RequestHandler takes parsed request as arguments (#QueryParam, #PathParam, #Body, etc)
 * The return value is automatically serialized and passed as the response.
 */
template<typename R, typename... Args>
using RequestHandler = R(const Args&...);

/**
 * Similar to RequestHandler, except that the request is processed asynchronously.
 */
template<typename R, typename... Args>
using AsyncRequestHandler = nhope::Future<R>(const Args&...);

template<typename T>
inline constexpr bool isRequstHandlerArg = isParam<T> || isBody<T>;

template<typename Handler>
inline constexpr bool isLowLevelHandler = std::is_invocable_v<Handler, std::shared_ptr<restbed::Session>>;

template<typename Fn, std::size_t... I>
constexpr bool checkFunctionArgs(std::index_sequence<I...> /*unused*/)
{
    using namespace nhope;
    using FnProps = FunctionProps<decltype(std::function(std::declval<Fn>()))>;
    return ((isRequstHandlerArg<std::decay_t<typename FnProps::template ArgumentType<I>>>)&&...);
}

template<typename R>
constexpr void checkRequestHandlerResult()
{
    static_assert(std::is_void_v<R> || detail::canSerializeJson<R>, "The handler result cannot be converted to json."
                                                                    "Please define a to_json function for it."
                                                                    "See https://github.com/nlohmann/json");
}

template<typename Handler, std::size_t... IArg>
auto callPrivateHandler(restbed::Session& session, Handler&& handler, std::index_sequence<IArg...> /*unused*/)
{
    using namespace nhope;
    using FnProps = FunctionProps<decltype(std::function(std::declval<Handler>()))>;
    using FnRet = typename FnProps::ReturnType;
    if constexpr (std::is_void_v<FnRet>) {
        handler(std::decay_t<typename FnProps::template ArgumentType<IArg>>(session)...);
        return;
    }
    return handler(std::decay_t<typename FnProps::template ArgumentType<IArg>>(session)...);
}

template<typename Handler>
constexpr void checkRequestHandler()
{
    using namespace nhope;
    using namespace std::literals;
    using FnProps = FunctionProps<decltype(std::function(std::declval<Handler>()))>;
    static_assert(checkFunctionArgs<Handler>(std::make_index_sequence<FnProps::argumentCount>{}),
                  "RequestHandler argument must be one of\n"
                  "\tParam <royalbed/param.h>)"
                  "\tBody <royalbed/body.h>");
    checkRequestHandlerResult<typename FnProps::ReturnType>();
}

template<typename Handler>
std::function<LowLevelHandler> makeLowLevelHandler(int status, Handler&& handler)
{
    using FnProps = nhope::FunctionProps<decltype(std::function(std::declval<Handler>()))>;
    using R = typename FnProps::ReturnType;
    checkRequestHandler<Handler>();

    return [status, handler = std::move(handler)](
             // NOLINTNEXTLINE(performance-unnecessary-value-param)
             std::shared_ptr<restbed::Session> session) {
        try {
            if constexpr (std::is_void_v<R>) {
                callPrivateHandler(*session, handler, std::make_index_sequence<FnProps::argumentCount>{});
                session->close(status);
            } else {
                R result = callPrivateHandler(*session, handler, std::make_index_sequence<FnProps::argumentCount>{});
                sendJson(*session, status, result);
            }
        } catch (...) {
            auto exPtr = std::current_exception();
            sendError(*session, std::move(exPtr));
        }
    };
}

}   // namespace royalbed
