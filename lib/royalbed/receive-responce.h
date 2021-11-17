#pragma once

#include "nhope/async/ao-context.h"
#include "nhope/async/future.h"
#include "nhope/io/pushback-reader.h"

#include "royalbed/detail/responce.h"

namespace royalbed::detail {

nhope::Future<ResponcePtr> receiveResponce(nhope::AOContext& aoCtx, nhope::PushbackReader& device);

}   // namespace royalbed::detail
