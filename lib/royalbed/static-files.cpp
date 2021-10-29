#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <string>

#include <corvusoft/restbed/byte.hpp>
#include <corvusoft/restbed/session.hpp>
#include <fmt/format.h>
#include <gsl/span>

#include "royalbed/http-status.h"
#include "royalbed/mime-type.h"
#include "royalbed/router.h"
#include "royalbed/static-files.h"

namespace royalbed {

namespace {
namespace fs = std::filesystem;
using namespace std::literals;

constexpr auto portionSize = 64 * 1024;

std::string join(std::list<std::string_view> args)
{
    args.remove_if([](auto a) {
        return a.empty();
    });
    return fmt::format("{}", fmt::join(args, "/"));
}

void sendNextPortion(const std::shared_ptr<restbed::Session>& session, gsl::span<const char> data)
{
    if (data.empty()) {
        session->close();
        return;
    }

    const auto nextPortionSize = std::min<std::size_t>(portionSize, data.size());
    const auto nextPortion = data.first(nextPortionSize);
    session->yield(restbed::Bytes{nextPortion.begin(), nextPortion.end()},
                   [remains = data.subspan(nextPortionSize)](const auto& session) {
                       sendNextPortion(session, remains);
                   });
}

void sendFirstPortion(const std::shared_ptr<restbed::Session>& session, gsl::span<const char> data)
{
    if (data.empty()) {
        session->close(HttpStatus::Ok);
        return;
    }

    const auto firstPortionSize = std::min<std::size_t>(portionSize, data.size());
    const auto firstPortion = data.first(firstPortionSize);
    session->yield(HttpStatus::Ok, restbed::Bytes{firstPortion.begin(), firstPortion.end()},
                   [remains = data.subspan(firstPortionSize)](const auto& session) {
                       sendNextPortion(session, remains);
                   });
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
    assert(entry.is_file());   // NOLINT

    const auto filePath = join({parentPath, entry.filename()});
    const auto file = fs.open(filePath);

    auto resourcePath = join({parentPath, entry.filename()});
    auto headers = std::multimap<std::string, std::string>{
      {"Content-Length", std::to_string(file.size())},
    };

    const auto contentEncoding = getContentEncodingByExtension(resourcePath);
    if (contentEncoding != std::nullopt) {
        headers.insert({"Content-Encoding", contentEncoding.value()});
        resourcePath = removeEncoderExtension(resourcePath);
    }

    const auto mimeType = mimeTypeForFileName(resourcePath);
    if (mimeType == std::nullopt) {
        const auto msg = fmt::format("Unable to determine a mime type for \"{}\"", filePath);
        throw std::runtime_error(msg);
    }
    headers.insert({"Content-Type", std::string(mimeType.value())});

    router.get(resourcePath, [file, headers](const auto& session) {
        session->set_headers(headers);
        sendFirstPortion(session, {file.begin(), file.end()});
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

Router staticFiles(const cmrc::embedded_filesystem& fs)
{
    Router router;
    for (const auto& entry : fs.iterate_directory("")) {
        publicDirEntry(router, fs, entry);
    }
    return router;
}

}   // namespace royalbed
