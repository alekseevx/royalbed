#include <filesystem>
#include <map>
#include <optional>
#include <string_view>
#include <string>

#include <royalbed/mime-type.h>

namespace royalbed {

namespace {
using namespace std::string_view_literals;

std::string getFileExt(std::string_view fileName)
{
    namespace fs = std::filesystem;
    return fs::path(fileName).extension();
}

}   // namespace

std::optional<std::string_view> mimeTypeForExt(std::string_view fileExt)
{
    static const auto mimeTypes = std::map<std::string_view, std::string_view>{
      {".json"sv, "application/json"sv},
      {".yml"sv, "application/yaml"sv},
      {".js"sv, "application/javascript"sv},
      {".bin"sv, "application/octet-stream"sv},
      {".png"sv, "image/png"sv},
      {".jpg"sv, "image/jpeg"sv},
      {".jpeg"sv, "image/jpeg"sv},
      {".html"sv, "text/html"sv},
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
