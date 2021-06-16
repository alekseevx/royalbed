#include <climits>
#include <limits>
#include <string>
#include <royalbed/string.h>
#include <gtest/gtest.h>
#include <type_traits>

namespace {

using namespace royalbed;
using namespace std::literals;

template<typename Int>
std::string tooBig()
{
    return std::to_string(std::numeric_limits<Int>::max()) + "1";
}

template<typename Int>
void checkFromString()   // NOLINT(readability-function-cognitive-complexity)
{
    EXPECT_ANY_THROW(fromString<Int>("TestString"s));   // NOLINT

    EXPECT_EQ(fromString<Int>("123"s), 123);

    EXPECT_EQ(fromString<Int>(std::to_string(std::numeric_limits<Int>::min())), std::numeric_limits<Int>::min());

    EXPECT_EQ(fromString<Int>(std::to_string(std::numeric_limits<Int>::max())), std::numeric_limits<Int>::max());
    EXPECT_ANY_THROW(fromString<Int>(tooBig<Int>()));   // NOLINT

    EXPECT_EQ(fromString<Int>("-123"s), static_cast<Int>(-123));   // NOLINT
}

}   // namespace

TEST(String, fromString)   // NOLINT
{
    // std::string
    {
        EXPECT_EQ(fromString<std::string>("TestString"s), "TestString"sv);
    }

    checkFromString<int>();
    checkFromString<unsigned int>();
    checkFromString<long>();
    checkFromString<unsigned long>();
    checkFromString<long long>();
    checkFromString<unsigned long long>();
}
