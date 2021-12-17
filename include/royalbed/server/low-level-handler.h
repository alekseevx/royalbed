#pragma once

#include <functional>

#include "nhope/utils/type.h"

#include "royalbed/server/request-context.h"

namespace royalbed::server {

using LowLevelHandler = std::function<nhope::Future<void>(RequestContext& ctx)>;

template<typename Handler>
static constexpr bool isLowLevelHandler = nhope::checkFunctionSignatureV<Handler, nhope::Future<void>, RequestContext&>;
template<typename Handler>
concept HightLevelHandler = !isLowLevelHandler<Handler> && nhope::isFunctional<Handler>();

}   // namespace royalbed::server
