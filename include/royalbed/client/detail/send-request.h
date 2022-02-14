#pragma once

#include <cstddef>

#include "nhope/async/ao-context.h"
#include "nhope/async/future.h"
#include "nhope/io/io-device.h"

#include "royalbed/client/request.h"

namespace royalbed::client::detail {

nhope::Future<std::size_t> sendRequest(nhope::AOContext& aoCtx, Request&& request, nhope::Writter& device);

}
