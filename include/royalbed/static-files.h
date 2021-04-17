#pragma once

#include <filesystem>
#include <string_view>

#include <cmrc/cmrc.hpp>
#include <royalbed/router.h>

namespace royalbed {

Router staticFiles(const cmrc::embedded_filesystem& fs);

}   // namespace royalbed
