#include <string>

#include <gtest/gtest.h>

#include "royalbed/common/detail/param-setters.h"
#include "royalbed/common/request.h"
#include <royalbed/common/detail/param.h>

namespace {

using namespace std::literals;
using namespace royalbed::common::detail;

}   // namespace

TEST(Param, makeProps)   // NOLINT
{
    constexpr auto testParam = "someTest"sv;
    {
        const auto pInt = makeProperties<int, NotRequired, DefaultInt<-4>>(testParam);
        EXPECT_EQ(pInt.name, testParam);
        EXPECT_FALSE(pInt.required);
        EXPECT_EQ(pInt.defaultValue.value(), -4);
        static constexpr auto defV{42};
        const auto p2Int = makeProperties<int, NotRequired, DefaultValue<defV>>(testParam);
        EXPECT_EQ(p2Int.defaultValue.value(), defV);
    }

    {
        const auto pStr = makeProperties<std::string, Required, DefaultStr<"X">>(testParam);
        EXPECT_EQ(pStr.name, testParam);
        EXPECT_EQ(pStr.defaultValue.value(), "X");
    }
}

TEST(Param, simple)   // NOLINT
{
    constexpr auto testParam = "someTest"sv;

    royalbed::common::Request req;
    req.uri.path = "12";

    PathParam<int, "someTest"> param(req);

    // EXPECT_EQ(param.get(), 12);
}
