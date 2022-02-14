#pragma once

#include <memory>
#include <string>

#include "nhope/io/io-device.h"

#include "royalbed/common/headers.h"
#include "royalbed/common/uri.h"

namespace royalbed::common {

struct Request final
{
    std::string method;
    Uri uri;
    Headers headers;
    nhope::ReaderPtr body;
};

}   // namespace royalbed::common
