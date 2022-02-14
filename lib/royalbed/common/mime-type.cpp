#include <filesystem>
#include <unordered_map>

#include "royalbed/common/mime-type.h"

namespace royalbed::common {

std::string getFileExt(std::string_view fileName)
{
    namespace fs = std::filesystem;
    return fs::path(fileName).extension();
}

std::string_view mimeTypeForExt(std::string_view fileExt)
{
    using namespace std::string_view_literals;
    static const auto mimeTypes = std::unordered_map<std::string_view, std::string_view>{
      {".json"sv, "application/json"sv},
      {".yml"sv, "application/yaml"sv},
      {".yaml"sv, "application/yaml"sv},
      {".js"sv, "application/javascript; charset=utf-8"sv},
      {".pdf"sv, "application/pdf"sv},
      {".zip"sv, "application/zip"sv},
      {".sql"sv, "application/sql"sv},
      {".xml"sv, "application/xml; charset=utf-8"sv},
      {".png"sv, "image/png"sv},
      {".jpg"sv, "image/jpeg"sv},
      {".gif"sv, "image/gif"sv},
      {".jpeg"sv, "image/jpeg"sv},
      {".html"sv, "text/html; charset=utf-8"sv},
      {".htm"sv, "text/html; charset=utf-8"sv},
      {".css"sv, "text/css; charset=utf-8"sv},
      {".csv"sv, "text/csv"sv},
      {".txt"sv, "text/plain; charset=utf-8"sv},
      {".svg"sv, "image/svg+xml"sv},
      {".wasm"sv, "application/wasm"sv},
    };

    const auto it = mimeTypes.find(fileExt);
    if (it == mimeTypes.end()) {
        return "application/octet-stream"sv;
    }

    return it->second;
}

std::string_view mimeTypeForFileName(std::string_view fileName)
{
    return mimeTypeForExt(getFileExt(fileName));
}

}   // namespace royalbed::common
