#include <string>

#include <gtest/gtest.h>
#include "royalbed/common/http-error.h"
#include "royalbed/server/detail/param-setters.h"
#include "spdlog/spdlog.h"

#include "nhope/async/ao-context.h"
#include "nhope/async/thread-executor.h"

#include "royalbed/server/detail/param.h"
#include "royalbed/common/request.h"
#include "royalbed/server/request-context.h"
#include "royalbed/server/router.h"

namespace {

using namespace std::literals;
using namespace royalbed::server::detail;
using namespace royalbed::server;

const std::string maxIntStr = std::to_string(std::numeric_limits<int>::max()) + "1";

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
    constexpr auto queryParam = "someTest2"sv;

    nhope::ThreadExecutor t;
    nhope::AOContext aoCtx(t);
    Router router;

    router.get("/prefix/:someTest/path2/", [](RequestContext& /*ctx*/) {
        return nhope::makeReadyFuture();
    });

    RequestContext reqCtx{
      .num = 1,
      .aoCtx = nhope::AOContext(aoCtx),
      .log = spdlog::default_logger(),
      .router = router,
      .request = {.uri = Uri::parseRelative("/prefix/42/path2/?someTest2=fx")},
    };
    const auto res = router.route("GET", reqCtx.request.uri.path);
    reqCtx.rawPathParams = res.rawPathParams;

    PathParam<int, "someTest"> param(reqCtx);
    PathParam<int, "notExists", NotRequired, DefaultInt<1>> defParam(reqCtx);
    QueryParam<std::string, "someTest2"> qParam(reqCtx);

    try {
        PathParam<int, "notExists", Required> throwParam(reqCtx);
        FAIL() << "must throw...";
    } catch (const royalbed::common::HttpError&) {
    }

    EXPECT_EQ(param.get(), 42);
    EXPECT_EQ(defParam.get(), 1);
    EXPECT_EQ(qParam.get(), "fx");
}

TEST(Param, invalid)   // NOLINT
{
    nhope::ThreadExecutor t;
    nhope::AOContext aoCtx(t);
    Router router;

    router.get("/prefix/:someTest/", [](RequestContext& /*ctx*/) {
        return nhope::makeReadyFuture();
    });

    RequestContext reqCtx{
      .num = 1,
      .aoCtx = nhope::AOContext(aoCtx),
      .log = spdlog::default_logger(),
      .router = router,
      .request = {.uri = Uri::parseRelative("/prefix/" + maxIntStr)},
    };
    const auto res = router.route("GET", reqCtx.request.uri.path);
    reqCtx.rawPathParams = res.rawPathParams;

    try {
        PathParam<int, "someTest"> param(reqCtx);
        FAIL() << "must throw...";
    } catch (const royalbed::common::HttpError&) {
    }
}
