#pragma once

#include <exception>
#include <optional>
#include <string>
#include <string_view>

#include "fmt/core.h"

#include "royalbed/common/detail/string-utils.h"
#include "royalbed/server/http-error.h"
#include "royalbed/server/http-status.h"
#include "royalbed/server/param-properties.h"
#include "royalbed/server/request-context.h"

namespace royalbed::server::detail {

std::optional<std::string> extractParam(const RequestContext& req, std::string_view name, ParamLocation loc,
                                        bool required);

template<typename T>
std::optional<T> extractParam(const RequestContext& req, const ParamProperties<T>& paramProps)
{
    const auto param = extractParam(req, paramProps.name, paramProps.loc, paramProps.required);
    try {
        if (!param.has_value()) {
            return paramProps.defaultValue;
        }

        T val = common::detail::fromString<T>(param.value());
        for (const auto& validate : paramProps.validators) {
            validate(val);
        }
        return val;

    } catch (const std::exception& ex) {
        const auto message = fmt::format("Failed to get '{}' parameter: {}", paramProps.name, ex.what());
        throw HttpError(HttpStatus::BadRequest, message);
    }
}

}   // namespace royalbed::server::detail
