#pragma once

#include <cstddef>

#include "nhope/async/ao-context.h"
#include "nhope/async/future.h"
#include "nhope/io/io-device.h"

#include "royalbed/detail/responce.h"

namespace royalbed::detail {

nhope::Future<std::size_t> sendResponce(nhope::AOContext& aoCtx, ResponcePtr responce, nhope::Writter& device);

}   // namespace royalbed::detail
