#pragma once

#include "nhope/async/ao-context.h"
#include "nhope/async/future.h"
#include "nhope/io/pushback-reader.h"

#include "royalbed/client/response.h"

namespace royalbed::client::detail {

nhope::Future<Response> receiveResponse(nhope::AOContext& aoCtx, nhope::PushbackReader& device);

}   // namespace royalbed::client::detail
