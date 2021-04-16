#include <algorithm>
#include <cstddef>
#include <memory>
#include <string_view>
#include <string>
#include <utility>

#include <corvusoft/restbed/resource.hpp>
#include <fmt/core.h>

#include <royalbed/router.h>

namespace royalbed {

namespace {
using namespace std::literals;
using PathResourceMap = std::map<std::string, std::shared_ptr<restbed::Resource>>;

std::string canonical(std::string_view path)
{
    std::string retval(path);

    auto pos = retval.find("//", 0);
    while (pos != std::string::npos) {
        retval.replace(pos, 2, "/");
        pos = retval.find("//", pos);
    }

    return retval;
}

std::string join(std::string_view p1, std::string_view p2)
{
    return fmt::format("{}/{}", p1, p2);
}

std::shared_ptr<restbed::Resource> getResource(const std::string& path, PathResourceMap& resources)
{
    auto& resourcePtr = resources[path];
    if (resourcePtr != nullptr) {
        return resourcePtr;
    }

    resourcePtr = std::make_shared<restbed::Resource>();
    resourcePtr->set_path(path);
    return resourcePtr;
}

bool startWith(std::string_view path, std::string_view prefix)
{
    return path.find(prefix) == 0;
}

}   // namespace

Router::Router(std::string_view prefix)
  : m_prefix(canonical(prefix))
{}

Router::~Router() = default;

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

Router& Router::use(std::string_view prefix, const Router& router)
{
    const auto fullPrefix = canonical(join(m_prefix, prefix));
    for (const auto& rec : router.m_handlerRecords) {
        const auto path = canonical(join(fullPrefix, rec.path));
        this->addHandler(rec.method, path, rec.handler);
    }
    return *this;
}

Router::Resources Router::resources() const
{
    PathResourceMap pathResourceMap;

    for (const auto& rec : m_handlerRecords) {
        const auto resource = getResource(rec.path, pathResourceMap);
        resource->set_method_handler(rec.method, rec.handler);
    }

    Resources retval;
    for (const auto& [_, resource] : pathResourceMap) {
        retval.emplace_back(resource);
    }

    return retval;
}

Router& Router::addHandler(std::string_view method, std::string_view path,
                           const std::function<LowLevelHandler>& handler)
{
    const auto fullPath = canonical(join(m_prefix, path));
    m_handlerRecords.push_back({std::string(method), fullPath, handler});
    return *this;
}

}   // namespace royalbed
