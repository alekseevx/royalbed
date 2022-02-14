#pragma once

#include <string>
#include "royalbed/common/headers.h"

namespace royalbed::common::detail {

void writeHeaders(const Headers& headers, std::string& out);

}
