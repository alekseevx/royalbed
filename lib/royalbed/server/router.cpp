#include <algorithm>
#include <cassert>
#include <cstddef>
#include <filesystem>
#include <memory>
#include <stop_token>
#include <string_view>
#include <string>
#include <system_error>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include "fmt/core.h"
#include "fmt/format.h"
#include "nhope/async/future.h"

#include "nhope/utils/scope-exit.h"
#include "royalbed/common/detail/string-utils.h"

#include "royalbed/server/low-level-handler.h"
#include "royalbed/server/middleware.h"
#include "royalbed/server/request-context.h"
#include "royalbed/server/responce.h"
#include "royalbed/server/router.h"
#include "royalbed/server/http-status.h"

namespace royalbed::server {

namespace {
using namespace royalbed::common::detail;

namespace fs = std::filesystem;
using namespace std::literals;
using namespace fmt::literals;

std::pair<std::string_view, std::string_view> headSegmentAndTail(std::string_view path) noexcept
{
    const auto pos = path.find('/');
    if (pos == std::string_view::npos) {
        return {path, ""sv};
    }
    return {path.substr(0, pos), path.substr(pos + 1)};
}

std::pair<std::string_view, std::string_view> headAndTailSegment(std::string_view path) noexcept
{
    const auto pos = path.rfind('/');
    if (pos == std::string_view::npos) {
        return {""sv, path};
    }
    return {path.substr(0, pos), path.substr(pos + 1)};
}

std::string_view limitPathByDepth(std::string_view path, std::size_t depth)
{
    // The path must be normalized (see normalizePath)
    assert(path.empty() || path[0] != '/');   // NOLINT

    if (depth == 1) {
        // The root
        return ""sv;
    }

    if (depth == 2) {
        // The first semgent
        assert(!path.empty());   // NOLINT
        return path.substr(0, path.find('/'));
    }

    std::size_t len = 0;
    for (; depth > 1; --depth) {
        // The path must be normalized (see normalizePath)
        assert(len != std::string_view::npos);   // NOLINT
        assert(len + 1 < path.size());           // NOLINT

        len = path.find('/', len + 1);
    }
    return path.substr(0, len);
}

bool isParamSegment(std::string_view segment) noexcept
{
    return !segment.empty() && segment[0] == ':';
}

bool isFixedSegment(std::string_view segment) noexcept
{
    return !isParamSegment(segment);
}

std::string normalizePath(std::string_view path)
{
    std::string retval = fs::path(path, fs::path::format::generic_format)   //
                           .lexically_normal()
                           .generic_string();

    if (retval.starts_with('/')) {
        retval.erase(0, 1);
    }
    if (retval.ends_with('/')) {
        retval.pop_back();
    }
    return retval;
}

const auto defaultNotFoundHandler = LowLevelHandler{[](RequestContext& ctx) {
    ctx.log->error("Resource route \"{}\" not found", ctx.request.uri.path);

    ctx.responce.status = HttpStatus::NotFound;
    ctx.responce.statusMessage = HttpStatus::message(HttpStatus::NotFound);

    return nhope::makeReadyFuture();
}};

const auto defaultMethodNotAllowedHandler = LowLevelHandler{[](RequestContext& ctx) {
    ctx.log->error("Method \"{}\" not allowed", ctx.request.method);

    const auto allowMethods = ctx.router.allowMethods(ctx.request.uri.path);

    ctx.responce.status = HttpStatus::MethodNotAllowed;
    ctx.responce.statusMessage = HttpStatus::message(HttpStatus::MethodNotAllowed);
    ctx.responce.headers["Allow"] = fmt::format("{}", fmt::join(allowMethods, ", "));

    return nhope::makeReadyFuture();
}};

const auto nullHandler = LowLevelHandler{nullptr};

}   // namespace

class Router::Node final
{
public:
    using MethodHandlers = std::unordered_map<std::string, LowLevelHandler, StringHash, StringEqual>;

    struct FindResult
    {
        bool found;

        const Node* bestNode;
        std::size_t bestNodeDepth;
    };

    Node(std::string_view segment = ""sv, const Node* parent = nullptr)
      : m_nodeSegment(segment)
      , m_parent(parent)
    {}

    [[nodiscard]] bool isRoot() const noexcept
    {
        return m_parent == nullptr;
    }

    [[nodiscard]] bool isParamNode() const noexcept
    {
        return isParamSegment(m_nodeSegment);
    }

    Node* findOrCreateIfNotExist(std::string_view resource)
    {
        if (resource.empty()) {
            return this;
        }

        const auto [headSegment, tail] = headSegmentAndTail(resource);
        auto& subtree = isFixedSegment(headSegment) ? m_fixedSubtree : m_paramSubtree;
        auto& ptr = subtree[std::string{headSegment}];
        if (ptr == nullptr) {
            ptr = std::make_unique<Node>(headSegment, this);
        }

        return ptr->findOrCreateIfNotExist(tail);
    }

    FindResult findNode(std::string_view path) const
    {
        FindCtx ctx;
        const bool found = this->findNode(ctx, path);
        return {found, ctx.bestNode, ctx.bestNodeDepth};
    }

    std::vector<std::string> allowMethods() const
    {
        auto methodNames = std::vector<std::string>{};
        methodNames.reserve(m_methodHandlers.size());
        for (const auto& [methodName, _] : m_methodHandlers) {
            methodNames.push_back(methodName);
        }
        return methodNames;
    }

    void setMethodHandler(std::string_view method, LowLevelHandler&& handler)
    {
        m_methodHandlers[std::string{method}] = std::move(handler);
    }

    const LowLevelHandler& findMethodHandler(std::string_view method) const
    {
        const auto iter = m_methodHandlers.find(std::string{method});
        if (iter == m_methodHandlers.end()) {
            return nullHandler;
        }
        return iter->second;
    }

    void setNotFoundHandler(LowLevelHandler&& handler)
    {
        m_notFoundHandler = std::move(handler);
    }

    const LowLevelHandler& notFoundHandler() const noexcept
    {
        return this->findBestErrHandler(&Node::m_notFoundHandler, defaultNotFoundHandler);
    }

    void setMethodNotAllowedHandler(LowLevelHandler&& handler)
    {
        m_methodNotAllowedHandler = std::move(handler);
    }

    const LowLevelHandler& methodNotAllowedHandler() const noexcept
    {
        return this->findBestErrHandler(&Node::m_methodNotAllowedHandler, defaultMethodNotAllowedHandler);
    }

    void addMiddleware(Middleware&& middleware)
    {
        m_middlewares.push_back(std::move(middleware));
    }

    std::list<Middleware> middlewares() const
    {
        std::list<Middleware> result;
        for (const Node* cur = this; cur != nullptr; cur = cur->m_parent) {
            result.insert(result.begin(), cur->m_middlewares.begin(), cur->m_middlewares.end());
        }
        return result;
    }

    void extractParamsFromPath(std::string_view path, RawPathParams& params) const
    {
        const auto [pathHead, pathTailSegment] = headAndTailSegment(path);
        if (!pathHead.empty()) {
            assert(m_parent != nullptr);   // NOLINT
            m_parent->extractParamsFromPath(pathHead, params);
        }

        if (this->isParamNode()) {
            params.emplace_back(m_nodeSegment, pathTailSegment);
        }
    }

    void takeHandlersAndMiddlewares(Node& other)
    {
        m_methodHandlers = std::move(other.m_methodHandlers);
        m_methodNotAllowedHandler = std::move(other.m_methodNotAllowedHandler);
        m_notFoundHandler = std::move(other.m_notFoundHandler);
        m_middlewares = std::move(other.m_middlewares);
    }

    template<typename Visitor>
    void visit(const Visitor& visitor)
    {
        std::string path;
        visit(visitor, path);
    }

private:
    struct FindCtx
    {
        void entry(const Node* node)
        {
            if (++curDepth > bestNodeDepth) {
                bestNode = node;
                bestNodeDepth = curDepth;
            }
        }

        void leave()
        {
            assert(curDepth > 1);   // NOLINT
            --curDepth;
        }

        std::size_t curDepth = 0;

        std::size_t bestNodeDepth = curDepth;
        const Node* bestNode = nullptr;
    };

    bool findNode(FindCtx& ctx, std::string_view path) const
    {
        ctx.entry(this);
        nhope::ScopeExit leaveScopeExit([&ctx] {
            ctx.leave();
        });

        if (path.empty()) {
            return true;
        }

        const auto [headSegment, tail] = headSegmentAndTail(path);
        if (const auto iter = m_fixedSubtree.find(std::string{headSegment}); iter != m_fixedSubtree.end()) {
            if (iter->second->findNode(ctx, tail)) {
                return true;
            }
        }

        for (const auto& [_, node] : m_paramSubtree) {
            if (node->findNode(ctx, tail)) {
                return true;
            }
        }

        return false;
    }

    template<typename Visitor>
    void visit(const Visitor& visitor, std::string& path)
    {
        const auto curPathLen = path.size();
        path += fmt::format("/{}", m_nodeSegment);

        visitor(path, *this);

        for (const auto& p : m_paramSubtree) {
            p.second->visit(visitor, path);
        }
        for (const auto& p : m_fixedSubtree) {
            p.second->visit(visitor, path);
        }

        path.resize(curPathLen);
    }

    const LowLevelHandler& findBestErrHandler(const LowLevelHandler Node::*handlerField,
                                              const LowLevelHandler& defaultHandler) const noexcept
    {
        for (const Node* cur = this; cur != nullptr; cur = cur->m_parent) {
            if (cur->*handlerField != nullptr) {
                return cur->*handlerField;
            }
        }
        return defaultHandler;
    }

    using Subtree = std::unordered_map<std::string, std::unique_ptr<Node>, StringHash, StringEqual>;

    const std::string m_nodeSegment;
    const Node* const m_parent;

    std::unordered_map<std::string, LowLevelHandler, StringHash, StringEqual> m_methodHandlers;
    LowLevelHandler m_notFoundHandler;
    LowLevelHandler m_methodNotAllowedHandler;

    std::list<Middleware> m_middlewares;

    Subtree m_fixedSubtree;
    Subtree m_paramSubtree;
};

Router::Router()
  : m_root(std::make_unique<Node>())
{}

Router::~Router() = default;

Router::Router(Router&&) noexcept = default;

Router& Router::operator=(Router&&) noexcept = default;

Router& Router::get(std::string_view resource, LowLevelHandler handler)
{
    return this->addRoute("GET"sv, resource, std::move(handler));
}

Router& Router::post(std::string_view resource, LowLevelHandler handler)
{
    return this->addRoute("POST"sv, resource, std::move(handler));
}

Router& Router::put(std::string_view resource, LowLevelHandler handler)
{
    return this->addRoute("PUT"sv, resource, std::move(handler));
}

Router& Router::patch(std::string_view resource, LowLevelHandler handler)
{
    return this->addRoute("PATCH"sv, resource, std::move(handler));
}

Router& Router::options(std::string_view resource, LowLevelHandler handler)
{
    return this->addRoute("OPTIONS"sv, resource, std::move(handler));
}

Router& Router::head(const std::string_view resource, LowLevelHandler handler)
{
    return this->addRoute("HEAD"sv, resource, std::move(handler));
}

Router& Router::addMiddleware(Middleware middleware)
{
    m_root->addMiddleware(std::move(middleware));
    return *this;
}

Router& Router::use(std::string_view prefix, Router&& router)
{
    router.m_root->visit([this, prefix](std::string_view path, Node& node) {
        const auto newPath = normalizePath(fmt::format("{}/{}", prefix, path));
        auto* newNode = m_root->findOrCreateIfNotExist(newPath);
        newNode->takeHandlersAndMiddlewares(node);
    });
    return *this;
}

Router& Router::setNotFoundHandler(LowLevelHandler handler)
{
    assert(handler != nullptr);   // NOLINT

    m_root->setNotFoundHandler(std::move(handler));
    return *this;
}

Router& Router::setMethodNotAllowedHandler(LowLevelHandler handler)
{
    assert(handler != nullptr);   // NOLINT

    m_root->setMethodNotAllowedHandler(std::move(handler));
    return *this;
}

RouteResult Router::route(std::string_view method, std::string_view path) const
{
    RouteResult result;

    const auto normalizedPath = normalizePath(path);
    const auto [found, bestNode, bestNodeDepth] = m_root->findNode(normalizedPath);
    assert(bestNode != nullptr);   // NOLINT

    if (!found) {
        result.handler = bestNode->notFoundHandler();
        return result;
    }

    result.handler = bestNode->findMethodHandler(method);
    if (result.handler == nullptr) {
        result.handler = bestNode->methodNotAllowedHandler();
        return result;
    }

    result.middlewares = bestNode->middlewares();
    const auto processedPath = limitPathByDepth(normalizedPath, bestNodeDepth);
    bestNode->extractParamsFromPath(processedPath, result.rawPathParams);

    return result;
}

std::vector<std::string> Router::allowMethods(std::string_view path) const
{
    const auto normalizedPath = normalizePath(path);
    const auto [found, bestNode, bestNodeDepth] = m_root->findNode(normalizedPath);
    if (!found) {
        return {};
    }

    return bestNode->allowMethods();
}

Router& Router::addRoute(std::string_view method, std::string_view resource, LowLevelHandler handler)
{
    assert(handler != nullptr);   // NOLINT

    auto* node = m_root->findOrCreateIfNotExist(normalizePath(resource));
    assert(node != nullptr);   // NOLINT

    node->setMethodHandler(method, std::move(handler));
    return *this;
}

}   // namespace royalbed::server
