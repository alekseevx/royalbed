#pragma once

#include <memory>
#include <string>

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

}   // namespace royalbed::common
