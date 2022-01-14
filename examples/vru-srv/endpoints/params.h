#pragma once

#include <string>
#include "royalbed/server/param.h"

namespace vru_srv {
using VruId = royalbed::server::PathParam<std::string, "vruId", royalbed::server::Required>;

}   // namespace vru_srv
