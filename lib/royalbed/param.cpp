#include <limits>
#include <optional>
#include <stdexcept>
#include <string>

#include <corvusoft/restbed/request.hpp>

#include <royalbed/param.h>

namespace royalbed {

namespace {

std::optional<std::string> getParam(const restbed::Request& req, const std::string& name, ParamLoc loc)
{
    switch (loc) {
    case ParamLoc::Path:
        if (req.has_path_parameter(name)) {
            return req.get_path_parameter(name);
        } else {
            return std::nullopt;
        }

    case ParamLoc::Query:
        if (req.has_query_parameter(name)) {
            return req.get_query_parameter(name);
        } else {
            return std::nullopt;
        }
    };
}

}   // namespace

std::optional<std::string> getParam(const restbed::Request& req, const std::string& name, ParamLoc loc, bool required)
{
    auto param = getParam(req, name, loc);
    if (param != std::nullopt) {
        return param;
    }

    if (required) {
        const auto message = fmt::format("Required parameter '{}' not fount", name);
        throw HttpError(restbed::BAD_REQUEST, message);
    }

    return std::nullopt;
}

}   // namespace royalbed
