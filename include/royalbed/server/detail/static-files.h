#pragma once

#include "royalbed/server/router.h"

namespace cmrc {
class embedded_filesystem;
}

namespace royalbed::server::detail {

Router staticFiles(const cmrc::embedded_filesystem& fs);

}   // namespace royalbed::server::detail
