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

template<>
nhope::Future<nlohmann::json> parseBody<nlohmann::json>(server::RequestContext& req)
{
    return nhope::readAll(*req.request.body).then(req.aoCtx, [](const auto& bodyData) {
        return nlohmann::json::parse(bodyData.begin(), bodyData.end());
    });
}

}   // namespace royalbed::common
