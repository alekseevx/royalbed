#pragma once

#include <functional>
#include "royalbed/server/request-context.h"

namespace royalbed::server {

using LowLevelHandler = std::function<nhope::Future<void>(RequestContext& ctx)>;

}   // namespace royalbed::server
