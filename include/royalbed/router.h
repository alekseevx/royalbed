#pragma once

#include <functional>
#include <list>
#include <map>
#include <memory>
#include <string_view>
#include <string>
#include <type_traits>
#include <utility>

#include <corvusoft/restbed/resource.hpp>
#include <corvusoft/restbed/session.hpp>
#include <corvusoft/restbed/status_code.hpp>

#include <nhope/async/ao-context.h>
#include <nhope/async/future.h>

#include <royalbed/body.h>
#include <royalbed/param.h>
#include <royalbed/handler.h>

namespace royalbed {

class Router final
{
public:
    using Resources = std::list<std::shared_ptr<restbed::Resource>>;
    using PathResourceMap = std::map<std::string, std::shared_ptr<restbed::Resource>>;

    explicit Router(std::string_view prefix = "/");
    ~Router();

    template<typename Handler>
    Router& get(std::string_view path, Handler&& handler)
    {
        return this->get(path, restbed::OK, std::forward<Handler>(handler));
    }

    template<typename Handler>
    Router& get(std::string_view path, int status, Handler&& handler)
    {
        auto lowLevelHandler = makeLowLevelHandler(status, std::forward<Handler>(handler));
        return this->get(path, std::move(lowLevelHandler));
    }

    Router& get(std::string_view path, const std::function<LowLevelHandler>& handler);

    template<typename Handler>
    Router& put(std::string_view path, Handler&& handler)
    {
        return this->put(path, restbed::OK, std::forward<Handler>(handler));
    }

    template<typename Handler>
    Router& put(std::string_view path, int status, Handler&& handler)
    {
        auto lowLevelHandler = makeLowLevelHandler(status, std::forward<Handler>(handler));
        return this->addHandler("PUT", path, std::move(lowLevelHandler));
    }

    Router& put(std::string_view path, const std::function<LowLevelHandler>& handler);

    template<typename Handler>
    Router& post(std::string_view path, Handler&& handler)
    {
        return this->post(path, restbed::CREATED, std::forward<Handler>(handler));
    }

    template<typename Handler>
    Router& post(std::string_view path, int status, Handler&& handler)
    {
        auto lowLevelHandler = makeLowLevelHandler(status, std::forward<Handler>(handler));
        return this->post(path, status, std::move(lowLevelHandler));
    }

    Router& post(std::string_view path, const std::function<LowLevelHandler>& handler);

    template<typename Handler>
    Router& del(std::string_view path, Handler&& handler)
    {
        return this->del(path, restbed::OK, std::forward<Handler>(handler));
    }

    template<typename Handler>
    Router& del(std::string_view path, int status, Handler&& handler)
    {
        auto lowLevelHandler = makeLowLevelHandler(status, std::forward<Handler>(handler));
        return this->del(path, std::move(lowLevelHandler));
    }

    Router& del(std::string_view path, const std::function<LowLevelHandler>& handler);

    Router route(std::string_view path);

    [[nodiscard]] Resources resources() const;

private:
    Router(std::string_view prefix, std::shared_ptr<PathResourceMap> resources);

    Router& addHandler(std::string_view method, std::string_view path, const std::function<LowLevelHandler>& handler);

private:
    const std::string m_prefix;
    std::shared_ptr<PathResourceMap> m_resourceStoarge;
};

}   // namespace royalbed
