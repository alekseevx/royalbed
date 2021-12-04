#include <gtest/gtest.h>

#include "royalbed/common/detail/param-setters.h"
#include <royalbed/common/detail/param.h>

namespace {

using namespace std::literals;
using namespace royalbed::common::detail;

}   // namespace

TEST(Param, simple)   // NOLINT
{
    static const ParamProperties<int> p = makeProperties<int, NotRequired>("some"sv);
    EXPECT_EQ(p.name, "some"sv);
    EXPECT_FALSE(p.required);
}