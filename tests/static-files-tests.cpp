#include <cstddef>
#include <cstdint>
#include <cstring>
#include <optional>
#include <random>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include <cmrc/cmrc.hpp>
#include <corvusoft/restbed/http.hpp>
#include <corvusoft/restbed/settings.hpp>
#include <gsl/span>

#include <royalbed/static-files.h>

#include "helpers/resp.h"
#include "helpers/req.h"
#include "helpers/test-service.h"
#include "royalbed/http-status.h"

namespace {
using namespace std::literals;
using namespace royalbed;

std::vector<char> genRandom(std::size_t size)
{
    static std::mt19937 gen;   // NOLINT(cert-msc32-c,cert-msc51-cpp)

    std::vector<char> retval;
    retval.reserve(size);
    for (std::size_t i = 0; i < size; ++i) {
        retval.push_back(static_cast<char>(gen()));
    }
    return retval;
}

const auto emptyFileData = std::vector<char>();

constexpr auto smallFileSize = 235;
const auto smallFileData = genRandom(smallFileSize);   // NOLINT(cert-err58-cpp)

constexpr auto bigFileSize = 2 * 1024 * 1024 + 235;
const auto bigFileData = genRandom(bigFileSize);   // NOLINT(cert-err58-cpp)

const auto encodedFileData = std::vector<char>();

cmrc::embedded_filesystem testFs()
{
    using namespace cmrc::detail;

    static auto rootDir = directory();
    static auto rootFod = file_or_directory{rootDir};
    static auto folder1 = rootDir.add_subdir("folder1");
    static auto folder2 = rootDir.add_subdir("folder2");

    static auto rootIndex = index_type{
      {"", &rootFod},
      {"folder1", &folder1.index_entry},
      {"folder2", &folder2.index_entry},
      {
        "empty-file.json",
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        rootDir.add_file("empty-file.json", emptyFileData.data(), emptyFileData.data() + emptyFileData.size()),
      },
      {
        "folder2/small-file.bin",
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        folder2.directory.add_file("small-file.bin", smallFileData.data(), smallFileData.data() + smallFileData.size()),
      },
      {
        "folder2/big-file.bin",
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        folder2.directory.add_file("big-file.bin", bigFileData.data(), bigFileData.data() + bigFileData.size()),
      },
      {
        "folder2/encoded-file.js.gz",
        folder2.directory.add_file("encoded-file.js.gz", encodedFileData.data(),
                                   // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                                   encodedFileData.data() + encodedFileData.size()),
      },

    };

    return cmrc::embedded_filesystem(rootIndex);
}

bool eq(gsl::span<const char> v1, gsl::span<const std::uint8_t> v2)
{
    if (v1.size() != v2.size()) {
        return false;
    }

    return memcmp(v1.data(), v2.data(), v2.size()) == 0;
}

}   // namespace

TEST(StaticFiles, getFiles)   // NOLINT
{
    const auto router = staticFiles(testFs());
    EXPECT_EQ(router.resources().size(), 4);

    TestService srv(router.resources());

    struct TestRec
    {
        const std::string path;
        const std::string etalonContentType;
        const gsl::span<const char> etalonData;
        const std::string contentEncoding;
    };
    const auto testRecs = std::vector<TestRec>{
      {"/empty-file.json", "application/json", emptyFileData},
      {"/folder2/small-file.bin", "application/octet-stream", smallFileData},
      {"/folder2/big-file.bin", "application/octet-stream", bigFileData},
      {"/folder2/encoded-file.js", "application/javascript", encodedFileData, "gzip"},
    };

    for (const auto& rec : testRecs) {
        const auto req = makeReq(rec.path, "GET");
        const auto resp = restbed::Http::sync(req);
        EXPECT_EQ(resp->get_status_code(), HttpStatus::Ok);
        EXPECT_EQ(resp->get_header("Content-Type", ""), rec.etalonContentType);
        EXPECT_EQ(resp->get_header("Content-Length", SIZE_MAX), rec.etalonData.size());
        EXPECT_EQ(resp->get_header("Content-Encoding", ""), rec.contentEncoding);

        const auto body = restbed::Http::fetch(rec.etalonData.size(), resp);
        EXPECT_TRUE(eq(rec.etalonData, body));
    }
}
