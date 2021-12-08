#include <string>
#include <string_view>

#include "nhope/async/future.h"
#include "nhope/io/io-device.h"

#include "royalbed/server/request-context.h"
#include <royalbed/common/body.h>
#include <royalbed/common/http-error.h>
#include <royalbed/common/http-status.h>

namespace royalbed::common {
using namespace std::string_view_literals;

nlohmann::json getJson(const std::vector<std::uint8_t>& bodyData)
{
    return nlohmann::json::parse(bodyData.begin(), bodyData.end());
}

BodyType extractBodyType(common::Request& req)
{
    const auto contentType = req.headers.at("Content-Type");
    if (contentType == "application/json"sv) {
        return BodyType::Json;
    }
    if (contentType == "text/plain"sv) {
        return BodyType::Plain;
    }
    if (contentType == "application/octet-stream"sv) {
        return BodyType::Stream;
    }
    return BodyType::NoBody;
}

}   // namespace royalbed::common
