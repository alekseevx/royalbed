#include <cstdint>
#include <string>
#include <string_view>

#include <corvusoft/restbed/request.hpp>

#include <gtest/gtest.h>

#include <royalbed/http-error.h>
#include <royalbed/param.h>

namespace {

using namespace royalbed;
using namespace std::string_view_literals;

constexpr auto idParamName = "id"sv;
using IdParam = PathParam<std::uint64_t, idParamName, Required>;

constexpr auto limitParamName = "limit"sv;
constexpr auto limitParamDefaultValue = 100;
using LimitParam = QueryParam<int, limitParamName, NotRequired, DefaultValue<limitParamDefaultValue>>;

constexpr auto statusParamName = "status"sv;
using StatusParam = QueryParam<std::string, statusParamName, NotRequired>;

}   // namespace

TEST(Param, RequiredPathParam_NotFount)   // NOLINT
{
    restbed::Request req;
    EXPECT_THROW(IdParam{req}, HttpError);   // NOLINT
}

TEST(Param, NotRequiredQueryParamWithDefaultValue_NotFount)   // NOLINT
{
    restbed::Request req;
    LimitParam limitParam;
    EXPECT_NO_THROW(limitParam = LimitParam(req));   // NOLINT

    EXPECT_TRUE(limitParam.hasValue());
    EXPECT_EQ(limitParam.get(), LimitParam::props().defaultValue);
}

TEST(Param, NotRequiredQueryParamWithDefaultValue_Valid)   // NOLINT
{
    restbed::Request req;
    req.set_query_parameter(LimitParam::props().name, "1000");

    LimitParam limitParam;
    EXPECT_NO_THROW(limitParam = LimitParam(req));   // NOLINT

    EXPECT_TRUE(limitParam.hasValue());
    EXPECT_EQ(limitParam.get(), 1000);
}

TEST(Param, NotRequiredQueryParamWithDefaultValue_NotValid)   // NOLINT
{
    restbed::Request req;
    req.set_query_parameter(LimitParam::props().name, "not-number");

    LimitParam limitParam;
    EXPECT_THROW(limitParam = LimitParam(req), HttpError);   // NOLINT
}

TEST(Param, NotRequiredQueryParamWithoutDefaultValue_NotFount)   // NOLINT
{
    restbed::Request req;
    StatusParam statusParam;
    EXPECT_NO_THROW(statusParam = StatusParam(req));   // NOLINT

    EXPECT_FALSE(statusParam.hasValue());
    EXPECT_ANY_THROW((void)statusParam.get());   // NOLINT
}
