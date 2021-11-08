#pragma once

#include <memory>
#include <string>

#include "nhope/io/io-device.h"
#include "royalbed/detail/dict.h"

namespace royalbed::detail {

struct Request;
using RequestPtr = std::unique_ptr<Request>;

struct Request final
{
    using Headers = Dict<DictKeyCaseInsensitive>;
    using Query = Dict<DictKeyCaseSensitive>;

    std::string method;
    std::string path;

    Headers headers;
    Query query;

    nhope::ReaderPtr body;

    static RequestPtr create();
};

}   // namespace royalbed::detail
