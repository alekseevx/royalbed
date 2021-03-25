#include <algorithm>
#include <filesystem>
#include <memory>
#include <string_view>
#include <string>
#include <utility>

#include <corvusoft/restbed/resource.hpp>
#include <fmt/core.h>

#include <royalbed/router.h>

namespace royalbed {

namespace {
using PathResourceMap = Router::PathResourceMap;

struct GetResourceResult
{
    bool newResource;
    std::shared_ptr<restbed::Resource> resource;
};

std::string canonical(std::string_view p)
{
    namespace fs = std::filesystem;
    return fs::canonical(p);
}

std::string join(std::string_view p1, std::string_view p2)
{
    return fmt::format("{}/{}", p1, p2);
}

GetResourceResult getResource(const std::string& path, PathResourceMap& resources)
{
    auto& resourcePtr = resources[path];
    if (resourcePtr != nullptr) {
        return {false, resourcePtr};
    }

    resourcePtr = std::make_shared<restbed::Resource>();
    return {true, resourcePtr};
}

bool startWith(std::string_view path, std::string_view prefix)
{
    return path.find(prefix) == 0;
}

}   // namespace

Router::Router(std::string_view prefix)
  : m_prefix(canonical(prefix))
  , m_resourceStoarge(std::make_shared<PathResourceMap>())
{}

Router::Router(std::string_view prefix, std::shared_ptr<PathResourceMap> resources)
  : m_prefix(canonical(prefix))
  , m_resourceStoarge(std::move(resources))
{}

Router::~Router() = default;

Router Router::route(std::string_view path)
{
    return Router(join(m_prefix, path), m_resourceStoarge);
}

Router& Router::get(std::string_view path, const std::function<LowLevelHandler>& handler)
{
    return this->addHandler("GET", path, handler);
}

Router& Router::put(std::string_view path, const std::function<LowLevelHandler>& handler)
{
    return this->addHandler("PUT", path, handler);
}

Router& Router::post(std::string_view path, const std::function<LowLevelHandler>& handler)
{
    return this->addHandler("POST", path, handler);
}

Router& Router::del(std::string_view path, const std::function<LowLevelHandler>& handler)
{
    return this->addHandler("DELETE", path, handler);
}

Router::Resources Router::resources() const
{
    Resources retval;
    for (const auto& [path, resource] : *m_resourceStoarge) {
        if (startWith(path, m_prefix)) {
            retval.push_back(resource);
        }
    }
    return retval;
}

Router& Router::addHandler(std::string_view method, std::string_view path,
                           const std::function<LowLevelHandler>& handler)
{
    const auto fullPath = join(m_prefix, path);
    auto [newResource, resource] = getResource(fullPath, *m_resourceStoarge);
    if (newResource) {
        resource->set_path(fullPath);
    }

    resource->set_method_handler(std::string(method), handler);

    return *this;
}

}   // namespace royalbed
