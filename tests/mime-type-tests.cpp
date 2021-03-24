#include <optional>
#include <string_view>

#include <royalbed/mime-type.h>

#include <gtest/gtest.h>

using namespace royalbed;
using namespace std::string_view_literals;

TEST(MimeType, FindByFileExt)   // NOLINT
{
    EXPECT_EQ(mimeTypeForExt("invalidExt"), std::nullopt);
    EXPECT_EQ(mimeTypeForExt(".js"), "text/javascript"sv);
}

TEST(MimeType, FindByFileName)   // NOLINT
{
    EXPECT_EQ(mimeTypeForFileName("test-file.invalidExt"), std::nullopt);
    EXPECT_EQ(mimeTypeForFileName("test-file."), std::nullopt);
    EXPECT_EQ(mimeTypeForFileName("test-file.html.js"), "text/javascript"sv);
}
