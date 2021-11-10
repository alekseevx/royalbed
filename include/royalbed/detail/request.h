#pragma once

#include <memory>
#include <string>

#include "nhope/io/io-device.h"
#include "royalbed/detail/dict.h"
#include "royalbed/detail/uri.h"

namespace royalbed::detail {

struct Request;
using RequestPtr = std::unique_ptr<Request>;

struct Request final
{
    using Headers = Dict<DictKeyCaseInsensitive>;

    std::string method;
    Uri uri;
    Headers headers;
    nhope::ReaderPtr body;

    static RequestPtr create();
};

}   // namespace royalbed::detail
