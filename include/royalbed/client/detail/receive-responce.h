#pragma once

#include "nhope/async/ao-context.h"
#include "nhope/async/future.h"
#include "nhope/io/pushback-reader.h"

#include "royalbed/client/responce.h"

namespace royalbed::client::detail {

nhope::Future<Responce> receiveResponce(nhope::AOContext& aoCtx, nhope::PushbackReader& device);

}   // namespace royalbed::client::detail
