#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <string>

#include <fmt/format.h>

#include <cmrc/cmrc.hpp>

#include "nhope/async/future.h"
#include "nhope/io/string-reader.h"

#include <royalbed/common/mime-type.h>
#include <royalbed/server/router.h>
#include <royalbed/server/detail/static-files.h>

namespace royalbed::server {

namespace {
namespace fs = std::filesystem;
using namespace std::literals;

std::string join(std::list<std::string_view> args)
{
    args.remove_if([](auto a) {
        return a.empty();
    });
    return fmt::format("{}", fmt::join(args, "/"));
}

std::optional<std::string> getContentEncodingByExtension(std::string_view filePath)
{
    const auto ext = fs::path(filePath).extension().string();
    if (ext == ".gz") {
        return "gzip";
    }

    return std::nullopt;
}

std::string removeEncoderExtension(std::string_view filePath)
{
    const auto encoderExtensionPos = filePath.rfind('.');
    return std::string(filePath.substr(0, encoderExtensionPos));
}

void publicFile(Router& router, const cmrc::embedded_filesystem& fs, const cmrc::directory_entry& entry,
                std::string_view parentPath)

{
    auto resourcePath = join({parentPath, entry.filename()});
    const auto file = fs.open(resourcePath);
    const auto contentEncoding = getContentEncodingByExtension(resourcePath);
    if (contentEncoding) {
        resourcePath = removeEncoderExtension(resourcePath);
    }

    router.get(resourcePath, [resourcePath, file, contentEncoding](RequestContext& ctx) mutable {
        ctx.responce.headers["Content-Length"] = std::to_string(file.size());
        ctx.responce.headers["Content-Type"] = common::mimeTypeForFileName(resourcePath);

        if (contentEncoding != std::nullopt) {
            ctx.responce.headers["Content-Encoding"] = contentEncoding.value();
        };
        ctx.responce.body = nhope::StringReader::create(ctx.aoCtx, {file.begin(), file.end()});
        return nhope::makeReadyFuture();
    });
}

void publicDirEntry(Router& router, const cmrc::embedded_filesystem& fs, const cmrc::directory_entry& entry,
                    std::string_view parentPath = "")
{
    if (entry.is_file()) {
        publicFile(router, fs, entry, parentPath);
        return;
    }

    const auto entryPath = join({parentPath, entry.filename()});
    for (const auto& subEntry : fs.iterate_directory(entryPath)) {
        publicDirEntry(router, fs, subEntry, entryPath);
    }
}

}   // namespace

Router detail::staticFiles(const cmrc::embedded_filesystem& fs)
{
    Router router;
    for (const auto& entry : fs.iterate_directory("")) {
        publicDirEntry(router, fs, entry);
    }
    return router;
}

}   // namespace royalbed::server
