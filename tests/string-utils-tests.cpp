#include <functional>
#include <limits>
#include <string>
#include <type_traits>

#include <gtest/gtest.h>

#include "royalbed/detail/string-utils.h"

namespace {

using namespace royalbed::detail;
using namespace std::literals;

template<typename Int>
std::string tooBig()
{
    return std::to_string(std::numeric_limits<Int>::max()) + "1";
}

template<typename Int>
std::string tooSmall()
{
    if constexpr (std::is_unsigned_v<Int>) {
        return "-1";
    } else {
        return std::to_string(std::numeric_limits<Int>::min()) + "1";
    }
}

template<typename Int>
void checkFromString()   // NOLINT(readability-function-cognitive-complexity)
{
    EXPECT_ANY_THROW(fromString<Int>("TestString"s));   // NOLINT

    EXPECT_EQ(fromString<Int>(" +123 "s), 123);

    EXPECT_EQ(fromString<Int>(std::to_string(std::numeric_limits<Int>::min())), std::numeric_limits<Int>::min());

    EXPECT_EQ(fromString<Int>(std::to_string(std::numeric_limits<Int>::max())), std::numeric_limits<Int>::max());
    EXPECT_ANY_THROW(fromString<Int>(tooBig<Int>()));     // NOLINT
    EXPECT_ANY_THROW(fromString<Int>(tooSmall<Int>()));   // NOLINT

    EXPECT_ANY_THROW(fromString<Int>("0.5"));   // NOLINT
}

}   // namespace

TEST(StringUtils, fromString)   // NOLINT
{
    // std::string
    {
        EXPECT_EQ(fromString<std::string>("TestString"s), "TestString"sv);
    }

    checkFromString<short>();
    checkFromString<unsigned short>();

    checkFromString<int>();
    checkFromString<unsigned int>();

    checkFromString<long>();
    checkFromString<unsigned long>();

    checkFromString<long long>();
    checkFromString<unsigned long long>();
}

TEST(StringUtils, toLower)   // NOLINT
{
    EXPECT_EQ(toLower("QwErtY"sv), "qwerty"sv);
}

TEST(StringUtils, LowercaseHash)   // NOLINT
{
    EXPECT_EQ(LowercaseHash{}("QwErtY"), std::hash<std::string>{}("qwerty"));
}

TEST(StringUtils, LowercaseEqual)   // NOLINT
{
    EXPECT_TRUE(LowercaseEqual{}("QwErtY", "qwerty"));
}

TEST(StringUtils, LowercaseLess)   // NOLINT
{
    EXPECT_TRUE(LowercaseLess{}("a", "B"));
}
