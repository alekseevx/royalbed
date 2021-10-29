#include <string>
#include <string_view>

#include <corvusoft/restbed/request.hpp>
#include <corvusoft/restbed/session.hpp>

#include "royalbed/body.h"
#include "royalbed/http-status.h"

namespace royalbed {
using namespace std::string_view_literals;

template<>
nlohmann::json parseBody<nlohmann::json>(const restbed::Request& req)
{
    const auto contentType = req.get_header("Content-Type");
    if (contentType != "application/json"sv) {
        throw HttpError(HttpStatus::UnsupportedMediaType, "Request body must be JSON");
    }

    try {
        const auto& body = req.get_body();
        return nlohmann::json::parse(body.begin(), body.end());
    } catch (const std::exception& ex) {
        const auto message = fmt::format("Failed to parse request body: {}", ex.what());
        throw HttpError(HttpStatus::BadRequest, message);
    }
}

}   // namespace royalbed
