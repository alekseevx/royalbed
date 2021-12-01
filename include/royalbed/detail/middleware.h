#pragma once

#include <functional>
#include "royalbed/detail/request-context.h"

namespace royalbed::detail {

using Middleware = std::function<nhope::Future<bool>(RequestContext& ctx)>;

}   // namespace royalbed::detail
