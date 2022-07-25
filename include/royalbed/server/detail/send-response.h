#pragma once

#include <cstddef>

#include "nhope/async/ao-context.h"
#include "nhope/async/future.h"
#include "nhope/io/io-device.h"

#include "royalbed/server/response.h"

namespace royalbed::server::detail {

nhope::Future<std::size_t> sendResponse(nhope::AOContext& aoCtx, Response&& response, nhope::Writter& device);

}   // namespace royalbed::server::detail
