#pragma once

#include "nhope/async/ao-context.h"
#include "nhope/async/future.h"
#include "nhope/io/pushback-reader.h"

#include "royalbed/server/request.h"

namespace royalbed::server::detail {

nhope::Future<Request> receiveRequest(nhope::AOContext& aoCtx, nhope::PushbackReader& device);

}   // namespace royalbed::server::detail
