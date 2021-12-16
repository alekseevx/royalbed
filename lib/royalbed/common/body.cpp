#include <string>
#include <string_view>

#include "nhope/async/future.h"
#include "nhope/io/io-device.h"

#include "royalbed/server/request-context.h"
#include <royalbed/common/body.h>
#include <royalbed/common/http-error.h>
#include <royalbed/common/http-status.h>

namespace royalbed::common {
namespace {

using namespace std::literals;
constexpr auto jsonContent{"application/json"sv};
constexpr auto plainContent{"text/plain"sv};
const auto content = "Content-Type"s;

}   // namespace

BodyType extractBodyType(const Headers& headers)
{
    const auto& contentType = headers.at(content);
    if (contentType == jsonContent) {
        return BodyType::Json;
    }
    if (contentType == plainContent) {
        return BodyType::Plain;
    }
    throw HttpError(HttpStatus::BadRequest, fmt::format("{0} \"{1}\" not supported yet", content, contentType));
}

}   // namespace royalbed::common
