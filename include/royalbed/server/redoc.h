#pragma once

#include <string_view>

namespace cmrc {
class embedded_filesystem;
}

namespace royalbed::server {

class Router;

/**
 * Добавляет endpint "/redoc" в router для доступа к redoc.
 * Докумен с описанием API будет доступен по пути "/redoc/doc-api" (относительно router-а).
 */
void redoc(Router& router, const cmrc::embedded_filesystem& fs, std::string_view openApiFilePath);

}   // namespace royalbed::server
