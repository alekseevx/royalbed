#pragma once

#include <cmrc/cmrc.hpp>
#include <royalbed/router.h>
#include <string_view>

namespace royalbed {

/**
 * Добавляет endpint "/swagger" в router для доступа к swagger.
 * Докумен с описанием API будет доступен по пути "/swagger/api.json" (относительно router-а).
 */
void swagger(Router& router, const cmrc::file& swaggerApiFile);
void swagger(Router& router, const cmrc::embedded_filesystem& fs, std::string_view swaggerApiFilePath);

}   // namespace royalbed
