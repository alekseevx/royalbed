#pragma once

#include <optional>
#include <string>

namespace royalbed {

std::optional<std::string_view> mimeTypeForExt(std::string_view fileExt);
std::optional<std::string_view> mimeTypeForFileName(std::string_view fileName);

}   // namespace royalbed
