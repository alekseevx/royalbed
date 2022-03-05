#pragma once

#include <string_view>

namespace cmrc {
class embedded_filesystem;
}

namespace royalbed::server {

class Router;

/**
 * Добавляет endpint "/swagger" в router для доступа к swagger.
 * Докумен с описанием API будет доступен по пути "/swagger/doc-api" (относительно router-а).
 */
void swagger(Router& router, const cmrc::embedded_filesystem& fs, std::string_view openApiFilePath);

}   // namespace royalbed::server
