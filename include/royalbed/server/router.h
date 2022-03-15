#pragma once

#include <exception>
#include <list>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include "nhope/async/ao-context.h"
#include "nhope/async/future.h"

#include "royalbed/common/http-status.h"
#include "royalbed/server/low-level-handler.h"
#include "royalbed/server/middleware.h"
#include "royalbed/server/request-context.h"
#include "royalbed/server/detail/handler.h"
#include "royalbed/server/string-literal.h"

namespace royalbed::server {

struct RouteResult final
{
    LowLevelHandler handler;
    std::list<Middleware> middlewares;
    RawPathParams rawPathParams;
};

class Router
{
public:
    Router();
    ~Router();

    Router(const Router&) = delete;
    Router& operator=(const Router&) = delete;

    Router(Router&&) noexcept;
    Router& operator=(Router&&) noexcept;

    Router& get(std::string_view resource, LowLevelHandler handler);
    Router& post(std::string_view resource, LowLevelHandler handler);
    Router& put(std::string_view resource, LowLevelHandler handler);
    Router& patch(std::string_view resource, LowLevelHandler handler);
    Router& options(std::string_view resource, LowLevelHandler handler);
    Router& head(std::string_view resource, LowLevelHandler handler);
    Router& del(std::string_view resource, LowLevelHandler handler);

    Router& addMiddleware(Middleware middleware);

    Router& use(std::string_view prefix, Router&& router);

    Router& setNotFoundHandler(LowLevelHandler handler);
    Router& setMethodNotAllowedHandler(LowLevelHandler handler);
    Router& setExceptionHandler(ExceptionHandler handler);

    [[nodiscard]] RouteResult route(std::string_view method, std::string_view path) const;
    [[nodiscard]] std::vector<std::string> allowMethods(std::string_view path) const;

    template<StringLiteral resource, HightLevelHandler Handler>
    constexpr Router& get(Handler&& handler, int statusCode = HttpStatus::Ok)
    {
        detail::checkResource<Handler, resource>();
        return this->get(resource, detail::makeLowLevelHandler(std::forward<Handler>(handler), statusCode));
    }

    template<HightLevelHandler Handler>
    constexpr Router& get(std::string_view resource, Handler&& handler, int statusCode = HttpStatus::Ok)
    {
        detail::checkResource<Handler>(resource);
        return this->get(resource, detail::makeLowLevelHandler(std::forward<Handler>(handler), statusCode));
    }

    template<StringLiteral resource, HightLevelHandler Handler>
    constexpr Router& post(Handler&& handler, int statusCode = HttpStatus::Created)
    {
        detail::checkResource<Handler, resource>();
        return this->post(resource, detail::makeLowLevelHandler(std::forward<Handler>(handler), statusCode));
    }

    template<HightLevelHandler Handler>
    constexpr Router& post(std::string_view resource, Handler&& handler, int statusCode = HttpStatus::Created)
    {
        detail::checkResource<Handler>(resource);
        return this->post(resource, detail::makeLowLevelHandler(std::forward<Handler>(handler), statusCode));
    }

    template<StringLiteral resource, HightLevelHandler Handler>
    constexpr Router& put(Handler&& handler, int statusCode = HttpStatus::Ok)
    {
        detail::checkResource<Handler, resource>();
        return this->put(resource, detail::makeLowLevelHandler(std::forward<Handler>(handler), statusCode));
    }

    template<HightLevelHandler Handler>
    constexpr Router& put(std::string_view resource, Handler&& handler, int statusCode = HttpStatus::Ok)
    {
        detail::checkResource<Handler>(resource);
        return this->put(resource, detail::makeLowLevelHandler(std::forward<Handler>(handler), statusCode));
    }

    template<StringLiteral resource, HightLevelHandler Handler>
    constexpr Router& patch(Handler&& handler, int statusCode = HttpStatus::Ok)
    {
        detail::checkResource<Handler, resource>();
        return this->patch(resource, detail::makeLowLevelHandler(std::forward<Handler>(handler), statusCode));
    }
    template<HightLevelHandler Handler>
    constexpr Router& patch(std::string_view resource, Handler&& handler, int statusCode = HttpStatus::Ok)
    {
        detail::checkResource<Handler>(resource);
        return this->patch(resource, detail::makeLowLevelHandler(std::forward<Handler>(handler), statusCode));
    }

    template<StringLiteral resource, HightLevelHandler Handler>
    constexpr Router& options(Handler&& handler, int statusCode = HttpStatus::Ok)
    {
        detail::checkResource<Handler, resource>();
        return this->options(resource, detail::makeLowLevelHandler(std::forward<Handler>(handler), statusCode));
    }

    template<HightLevelHandler Handler>
    constexpr Router& options(std::string_view resource, Handler&& handler, int statusCode = HttpStatus::Ok)
    {
        detail::checkResource<Handler>(resource);
        return this->options(resource, detail::makeLowLevelHandler(std::forward<Handler>(handler), statusCode));
    }

    template<HightLevelHandler Handler>
    constexpr Router& head(std::string_view resource, Handler&& handler, int statusCode = HttpStatus::Ok)
    {
        detail::checkResource<Handler>(resource);
        return this->head(resource, detail::makeLowLevelHandler(resource, std::forward<Handler>(handler), statusCode));
    }

    template<HightLevelHandler Handler>
    constexpr Router& del(std::string_view resource, Handler&& handler, int statusCode = HttpStatus::Ok)
    {
        detail::checkResource<Handler>(resource);
        return this->del(resource, detail::makeLowLevelHandler(std::forward<Handler>(handler), statusCode));
    }

    template<StringLiteral resource, HightLevelHandler Handler>
    constexpr Router& del(Handler&& handler, int statusCode = HttpStatus::Ok)
    {
        detail::checkResource<Handler, resource>();
        return this->del(resource, detail::makeLowLevelHandler(std::forward<Handler>(handler), statusCode));
    }

    [[nodiscard]] std::vector<std::string> resources() const;

private:
    Router& addRoute(std::string_view method, std::string_view resource, LowLevelHandler handler);
    void processException(RequestContext& ctx, std::exception_ptr e, std::string_view normalizedPath) const;

    class Node;
    std::unique_ptr<Node> m_root;
};

}   // namespace royalbed::server
