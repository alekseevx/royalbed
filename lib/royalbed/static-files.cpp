#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <string>

#include <corvusoft/restbed/byte.hpp>
#include <corvusoft/restbed/session.hpp>
#include <corvusoft/restbed/status_code.hpp>
#include <corvusoft/restbed/status_code.hpp>
#include <fmt/format.h>
#include <gsl/span>

#include <royalbed/mime-type.h>
#include <royalbed/router.h>
#include <royalbed/static-files.h>

namespace royalbed {

namespace {
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
        session->close(restbed::OK);
        return;
    }

    const auto firstPortionSize = std::min<std::size_t>(portionSize, data.size());
    const auto firstPortion = data.first(firstPortionSize);
    session->yield(restbed::OK, restbed::Bytes{firstPortion.begin(), firstPortion.end()},
                   [remains = data.subspan(firstPortionSize)](const auto& session) {
                       sendNextPortion(session, remains);
                   });
}

void publicFile(Router& router, const cmrc::embedded_filesystem& fs, const cmrc::directory_entry& entry,
                std::string_view parentPath)

{
    assert(entry.is_file());   // NOLINT

    const auto filePath = join({parentPath, entry.filename()});
    const auto mimeType = mimeTypeForFileName(filePath);
    if (mimeType == std::nullopt) {
        const auto msg = fmt::format("Unable to determine a mime type for \"{}\"", filePath);
        throw std::runtime_error(msg);
    }

    const auto file = fs.open(filePath);
    const auto resourcePath = join({parentPath, entry.filename()});
    router.get(resourcePath, [file, mimeType](const auto& session) {
        session->set_headers({
          {"Content-Type", std::string(mimeType.value())},
          {"Content-Length", std::to_string(file.size())},
        });

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
