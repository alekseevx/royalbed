#pragma once

#include <memory>
#include <string>
#include <string_view>

#include "nhope/async/ao-context.h"
#include "nhope/io/io-device.h"
#include "royalbed/common/headers.h"
#include "royalbed/common/http-status.h"

namespace royalbed::common {

struct Response final
{
    int status = HttpStatus::Ok;
    std::string statusMessage;
    Headers headers;
    nhope::ReaderPtr body;
};

Response makePlainTextResponse(nhope::AOContext& ctx, int status, std::string_view msg);

}   // namespace royalbed::common
