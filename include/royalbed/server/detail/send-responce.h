#pragma once

#include <cstddef>

#include "nhope/async/ao-context.h"
#include "nhope/async/future.h"
#include "nhope/io/io-device.h"

#include "royalbed/server/responce.h"

namespace royalbed::server::detail {

nhope::Future<std::size_t> sendResponce(nhope::AOContext& aoCtx, Responce&& responce, nhope::Writter& device);

}   // namespace royalbed::server::detail
