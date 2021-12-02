#pragma once

#include <functional>
#include "royalbed/server/request-context.h"

namespace royalbed::server {

using Middleware = std::function<nhope::Future<bool>(RequestContext& ctx)>;

}   // namespace royalbed::server
