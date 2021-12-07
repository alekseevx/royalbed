#pragma once

#include <exception>
#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <type_traits>

#include <nhope/async/ao-context.h>
#include <nhope/async/future.h>
#include <nhope/utils/type.h>

#include "nhope/io/io-device.h"
#include "nlohmann/json.hpp"
#include "royalbed/server/param.h"
#include "royalbed/server/low-level-handler.h"
#include "royalbed/common/detail/traits.h"
#include "royalbed/common/body.h"
#include "royalbed/server/request-context.h"

namespace royalbed::server::detail {

template<typename T>
static constexpr bool isRequstHandlerArg = isQueryOrParam<T> || common::isBody<T> || std::same_as<T, RequestContext>;

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
    static_assert(std::is_void_v<R> || common::detail::canSerializeJson<R>,
                  "The handler result cannot be converted to json."
                  "Please define a to_json function for it."
                  "See https://github.com/nlohmann/json");
}

template<typename Handler>
constexpr void checkRequestHandler()
{
    using namespace nhope;
    using FnProps = FunctionProps<decltype(std::function(std::declval<Handler>()))>;
    static_assert(checkFunctionArgs<Handler>(std::make_index_sequence<FnProps::argumentCount>{}),
                  "RequestHandler argument must be one of\n"
                  "\tParam <royalbed/server/param.h>)"
                  "\tBody <royalbed/common/body.h>");
    using R = typename FnProps::ReturnType;
    if constexpr (isFuture<R>) {
        checkRequestHandlerResult<typename R::Type>();
    } else {
        checkRequestHandlerResult<R>();
    }
}

template<typename Handler, std::size_t... IArg>
auto callPrivateHandler(RequestContext& ctx, Handler&& handler, std::index_sequence<IArg...> /*unused*/)
{
    using namespace nhope;
    using FnProps = FunctionProps<decltype(std::function(std::declval<Handler>()))>;
    using FnRet = typename FnProps::ReturnType;
    if constexpr (std::is_void_v<FnRet>) {
        handler(std::forward<typename FnProps::template ArgumentType<IArg>>(ctx)...);
        return;
    } else {
        return handler(std::forward<typename FnProps::template ArgumentType<IArg>>(ctx)...);
    }
}

void addContent(RequestContext& ctx, std::string content);

template<typename Handler>
LowLevelHandler makeLowLevelHandler(Handler&& handler)
{
    checkRequestHandler<Handler>();
    using FnProps = nhope::FunctionProps<decltype(std::function(std::declval<Handler>()))>;
    using R = typename FnProps::ReturnType;

    return [handler = std::move(handler)](RequestContext& ctx) {
        if constexpr (std::is_void_v<R>) {
            callPrivateHandler(ctx, handler, std::make_index_sequence<FnProps::argumentCount>{});
        } else {
            R result = callPrivateHandler(ctx, handler, std::make_index_sequence<FnProps::argumentCount>{});
            if constexpr (nhope::isFuture<R>) {
                using FR = typename R::Type;
                if constexpr (std::is_void_v<FR>) {
                    return result;
                } else {
                    return result.then(ctx.aoCtx, [&ctx](auto v) mutable {
                        addContent(ctx, nlohmann::to_string(nlohmann::json(v)));
                    });
                }
            } else {
                addContent(ctx, nlohmann::to_string(nlohmann::json(result)));
            }
        }
        return nhope::makeReadyFuture();
    };
}

}   // namespace royalbed::server::detail
