
#include "nhope/io/io-device.h"
#include "nhope/io/string-reader.h"
#include "royalbed/detail/request.h"

namespace royalbed::detail {

RequestPtr Request::create()
{
    return std::make_unique<Request>();
}

}   // namespace royalbed::detail
