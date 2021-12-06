
#include "royalbed/common/detail/param.h"

namespace royalbed::common::detail {

namespace {

std::optional<std::string> getParam(const common::Request& req, const std::string& name, ParamLocation loc)
{
    //TODO ...
    if (loc == ParamLocation::Path) {
        return req.uri.path;
    }
    return req.uri.query.front().second;
}
}   // namespace

std::optional<std::string> getParam(const Request& req, const std::string& name, ParamLocation loc, bool required)
{
    auto param = getParam(req, name, loc);
    if (param != std::nullopt) {
        return param;
    }

    if (required) {
        const auto message = fmt::format("Required parameter '{}' not fount", name);
        throw HttpError(HttpStatus::BadRequest, message);
    }

    return std::nullopt;
}

}   // namespace royalbed::common::detail
