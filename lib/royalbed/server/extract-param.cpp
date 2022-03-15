
#include <algorithm>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "fmt/core.h"

#include "royalbed/server/param.h"
#include "royalbed/server/error.h"
#include "royalbed/server/request-context.h"

namespace royalbed::server::detail {

namespace {

std::optional<std::string> findByName(const std::vector<std::pair<std::string, std::string>>& v, std::string_view name)
{
    const auto it = std::find_if(v.begin(), v.end(), [&](const auto& p) {
        return p.first == name;
    });
    if (it == v.end()) {
        return std::nullopt;
    }
    return it->second;
}

std::optional<std::string> extractParam(const RequestContext& req, std::string_view name, ParamLocation loc)
{
    if (loc == ParamLocation::Path) {
        return findByName(req.rawPathParams, name);
    }
    return findByName(req.request.uri.query, name);
}

}   // namespace

std::optional<std::string> extractParam(const RequestContext& req, const std::string_view name, ParamLocation loc,
                                        bool required)
{
    auto param = extractParam(req, name, loc);
    if (param != std::nullopt) {
        return param;
    }

    if (required) {
        const auto message = fmt::format("Required parameter '{}' not found", name);
        throw HttpError(HttpStatus::BadRequest, message);
    }

    return std::nullopt;
}

}   // namespace royalbed::server::detail
