#pragma once

#include <list>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include "nhope/async/ao-context.h"
#include "nhope/async/future.h"

#include "royalbed/server/low-level-handler.h"
#include "royalbed/server/middleware.h"
#include "royalbed/server/request-context.h"
#include "royalbed/server/detail/handler.h"

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

    Router& addMiddleware(Middleware middleware);

    Router& use(std::string_view prefix, Router&& router);

    Router& setNotFoundHandler(LowLevelHandler handler);
    Router& setMethodNotAllowedHandler(LowLevelHandler handler);

    [[nodiscard]] RouteResult route(std::string_view method, std::string_view path) const;
    [[nodiscard]] std::vector<std::string> allowMethods(std::string_view path) const;

    template<HightLevelHandler Handler>
    Router& get(std::string_view resource, Handler&& handler)
    {
        return this->get(resource, detail::makeLowLevelHandler(std::forward<Handler>(handler)));
    }

    template<HightLevelHandler Handler>
    Router& post(std::string_view resource, Handler&& handler)
    {
        return this->post(resource, detail::makeLowLevelHandler(std::forward<Handler>(handler)));
    }

    template<HightLevelHandler Handler>
    Router& put(std::string_view resource, Handler&& handler)
    {
        return this->put(resource, detail::makeLowLevelHandler(std::forward<Handler>(handler)));
    }

    template<HightLevelHandler Handler>
    Router& patch(std::string_view resource, Handler&& handler)
    {
        return this->patch(resource, detail::makeLowLevelHandler(std::forward<Handler>(handler)));
    }

    template<HightLevelHandler Handler>
    Router& options(std::string_view resource, Handler&& handler)
    {
        return this->options(resource, detail::makeLowLevelHandler(std::forward<Handler>(handler)));
    }

    template<HightLevelHandler Handler>
    Router& head(std::string_view resource, Handler&& handler)
    {
        return this->head(resource, detail::makeLowLevelHandler(std::forward<Handler>(handler)));
    }

private:
    Router& addRoute(std::string_view method, std::string_view resource, LowLevelHandler handler);

    class Node;
    std::unique_ptr<Node> m_root;
};

}   // namespace royalbed::server
