#pragma once

#include <functional>
#include "royalbed/detail/request-context.h"

namespace royalbed::detail {

using LowLevelHandler = std::function<nhope::Future<void>(RequestContext& ctx)>;

}   // namespace royalbed::detail
