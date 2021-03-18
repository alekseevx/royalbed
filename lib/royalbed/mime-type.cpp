#include <map>
#include <optional>
#include <string>
#include <string_view>

#include <royalbed/mime-type.h>

namespace royalbed {

namespace {
using namespace std::string_view_literals;

std::string_view getFileExt(std::string_view fileName)
{
    std::string_view retval;

    const auto p = fileName.find_last_of('.');
    if (p != std::string_view::npos) {
        retval = fileName.substr(p + 1);
    }

    return retval;
}

}   // namespace

std::optional<std::string_view> mimeTypeForExt(std::string_view fileExt)
{
    static const auto mimeTypes = std::multimap<std::string_view, std::string_view>{
      {"json"sv, "text/json"sv},
      {"yml"sv, "text/yaml"sv},
      {"js"sv, "text/javascript"sv},
    };

    const auto it = mimeTypes.find(fileExt);
    if (it == mimeTypes.end()) {
        return std::nullopt;
    }

    return it->second;
}

std::optional<std::string_view> mimeTypeForFileName(std::string_view fileName)
{
    const auto fileExt = getFileExt(fileName);
    return mimeTypeForExt(fileExt);
}

}   // namespace royalbed
