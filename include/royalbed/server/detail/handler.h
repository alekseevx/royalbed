#pragma once

#include <exception>
#include <functional>
#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <type_traits>

#include <nhope/async/ao-context.h>
#include <nhope/async/future.h>
#include <nhope/utils/type.h>

#include "nhope/io/io-device.h"
#include "nlohmann/json.hpp"
#include "royalbed/common/request.h"
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

template<typename... Params>
auto constructParams(Params&&... p)
{
    return std::make_tuple(std::forward<Params>(p)...);
}

template<BodyTypename BType, typename Type>
auto initParam(RequestContext& ctx, const BType& body)
{
    if constexpr (common::isBody<Type>) {
        return Type(body);
    } else if constexpr (std::is_constructible_v<Type, RequestContext&>) {
        return Type(ctx);
    } else {
        return Type{};
    }
}

template<typename Handler, BodyTypename BodyT, std::size_t... IArg>
auto callPrivateHandler(RequestContext& ctx, const BodyT& body, Handler&& handler,
                        std::index_sequence<IArg...> /*unused*/)
{
    using namespace nhope;
    using FnProps = FunctionProps<decltype(std::function(std::declval<Handler>()))>;
    using FnRet = typename FnProps::ReturnType;
    auto paramTuple = constructParams(
      initParam<decltype(body), std::forward<typename FnProps::template ArgumentType<IArg>>(ctx, body)...>);
    if constexpr (std::is_void_v<FnRet>) {
        std::apply(handler, std::move(paramTuple));
    } else {
        return std::apply(handler, std::move(paramTuple));
    }
}

void addContent(RequestContext& ctx, std::string content);

template<typename Handler>
LowLevelHandler makeLowLevelHandler(Handler&& handler)
{
    checkRequestHandler<Handler>();
    using FnProps = nhope::FunctionProps<decltype(std::function(std::declval<Handler>()))>;
    using R = typename FnProps::ReturnType;

    return [handler = std::move(handler)](RequestContext& ctx) mutable {
        return nhope::readAll(*ctx.request.body)
          .then(ctx.aoCtx, [&ctx, handler = std::move(handler)](const auto& rawBody) mutable {
              auto body = common::Body<int>(12);   // common::parseBody(ctx, rawBody);

              if constexpr (std::is_void_v<R>) {
                  callPrivateHandler(ctx, body, handler, std::make_index_sequence<FnProps::argumentCount>{});
              } else {
                  R result = callPrivateHandler(ctx, body, handler, std::make_index_sequence<FnProps::argumentCount>{});
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
              return;
          });
    };
}

}   // namespace royalbed::server::detail
