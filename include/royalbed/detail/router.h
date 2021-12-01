#pragma once

#include <list>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "nhope/async/ao-context.h"
#include "nhope/async/future.h"

#include "royalbed/detail/low-level-handler.h"
#include "royalbed/detail/middleware.h"
#include "royalbed/detail/request-context.h"

namespace royalbed::detail {

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

    Router& get(std::string_view resource, LowLevelHandler handler);
    Router& post(std::string_view resource, LowLevelHandler handler);
    Router& put(std::string_view resource, LowLevelHandler handler);
    Router& patch(std::string_view resource, LowLevelHandler handler);
    Router& options(std::string_view resource, LowLevelHandler handler);
    Router& head(const std::string& resource, LowLevelHandler handler);

    Router& use(std::string_view prefix, Router router);

    Router& setNotFoundHandler(LowLevelHandler handler);
    Router& setMethodNotAllowedHandler(LowLevelHandler handler);

    [[nodiscard]] RouteResult route(std::string_view method, std::string_view path) const;
    [[nodiscard]] std::vector<std::string> allowMethods(std::string_view path) const;

private:
    Router& addRoute(std::string_view method, std::string_view resource, LowLevelHandler handler);

    class Node;
    std::unique_ptr<Node> m_root;
};

}   // namespace royalbed::detail
