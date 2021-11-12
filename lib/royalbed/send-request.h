#pragma once

#include <cstddef>

#include "nhope/async/ao-context.h"
#include "nhope/async/future.h"
#include "nhope/io/io-device.h"

#include "royalbed/detail/request.h"

namespace royalbed::detail {

nhope::Future<std::size_t> sendRequest(nhope::AOContext& aoCtx, RequestPtr request, nhope::Writter& device);

}
