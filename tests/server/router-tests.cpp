#include <set>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "nhope/async/ao-context.h"
#include "nhope/async/future.h"
#include "nhope/async/thread-executor.h"
#include "nhope/io/io-device.h"
#include "nhope/io/string-reader.h"
#include "nhope/utils/type.h"

#include "nlohmann/json.hpp"
#include "royalbed/common/body.h"
#include "royalbed/server/low-level-handler.h"
#include "royalbed/server/middleware.h"
#include "royalbed/server/request-context.h"
#include "royalbed/server/request.h"
#include "royalbed/server/router.h"
#include "royalbed/server/param.h"

namespace {

using namespace std::literals;
using namespace royalbed::server;

}   // namespace

TEST(Router, MethodHandlerForRoot)   // NOLINT
{
    constexpr auto getMethod = [](RequestContext& /*ctx*/) {
        return nhope::makeReadyFuture();
    };
    constexpr auto postMethod = [](RequestContext& /*ctx*/) {
        return nhope::makeReadyFuture();
    };
    constexpr auto putMethod = [](RequestContext& /*ctx*/) {
        return nhope::makeReadyFuture();
    };
    constexpr auto patchMethod = [](RequestContext& /*ctx*/) {
        return nhope::makeReadyFuture();
    };
    constexpr auto optionsMethod = [](RequestContext& /*ctx*/) {
        return nhope::makeReadyFuture();
    };
    constexpr auto headMethod = [](RequestContext& /*ctx*/) {
        return nhope::makeReadyFuture();
    };

    Router router;
    router
      .get("/", getMethod)   //
      .post("/", postMethod)
      .put("/", putMethod)
      .patch("/", patchMethod)
      .options("/", optionsMethod)
      .head("/", headMethod);

    EXPECT_EQ(router.route("GET", "/").handler.target_type(), typeid(getMethod));
    EXPECT_EQ(router.route("POST", "/").handler.target_type(), typeid(postMethod));
    EXPECT_EQ(router.route("PUT", "/").handler.target_type(), typeid(putMethod));
    EXPECT_EQ(router.route("PATCH", "/").handler.target_type(), typeid(patchMethod));
    EXPECT_EQ(router.route("OPTIONS", "/").handler.target_type(), typeid(optionsMethod));
    EXPECT_EQ(router.route("HEAD", "/").handler.target_type(), typeid(headMethod));
}

TEST(Router, AddResources)   // NOLINT
{
    constexpr auto root = [](RequestContext& /*ctx*/) {
        return nhope::makeReadyFuture();
    };
    constexpr auto a = [](RequestContext& /*ctx*/) {
        return nhope::makeReadyFuture();
    };
    constexpr auto b = [](RequestContext& /*ctx*/) {
        return nhope::makeReadyFuture();
    };
    constexpr auto c = [](RequestContext& /*ctx*/) {
        return nhope::makeReadyFuture();
    };
    constexpr auto d = [](RequestContext& /*ctx*/) {
        return nhope::makeReadyFuture();
    };
    constexpr auto e = [](RequestContext& /*ctx*/) {
        return nhope::makeReadyFuture();
    };
    constexpr auto f = [](RequestContext& /*ctx*/) {
        return nhope::makeReadyFuture();
    };

    Router router;
    router.get("/", root);
    router.get("/aaaa", a);
    router.get("/aaaa/cc", c);
    router.get("/aaaa/dd", d);
    router.get("/bbbb/ee/f", f);
    router.get("/bbbb/ee", e);
    router.get("/bbbb", b);

    EXPECT_EQ(router.route("GET", "/").handler.target_type(), typeid(root));
    EXPECT_EQ(router.route("GET", "/aaaa").handler.target_type(), typeid(a));
    EXPECT_EQ(router.route("GET", "/bbbb").handler.target_type(), typeid(b));
    EXPECT_EQ(router.route("GET", "/aaaa/cc").handler.target_type(), typeid(c));
    EXPECT_EQ(router.route("GET", "/aaaa/dd").handler.target_type(), typeid(d));
    EXPECT_EQ(router.route("GET", "/bbbb/ee").handler.target_type(), typeid(e));
    EXPECT_EQ(router.route("GET", "/bbbb/ee/f").handler.target_type(), typeid(f));
}

TEST(Router, AddResourcesWithParams)   // NOLINT
{
    constexpr auto root = [](RequestContext& /*ctx*/) {
        return nhope::makeReadyFuture();
    };
    constexpr auto a = [](RequestContext& /*ctx*/) {
        return nhope::makeReadyFuture();
    };
    constexpr auto b = [](RequestContext& /*ctx*/) {
        return nhope::makeReadyFuture();
    };
    constexpr auto c = [](RequestContext& /*ctx*/) {
        return nhope::makeReadyFuture();
    };

    Router router;
    router.get("/", root);
    router.get("/prefix/:id/:repo/aaaaa", a);
    router.get("/prefix/:name/:project/bbbbbb", b);
    router.get("/prefix/ccccc/bbbbbb", c);

    EXPECT_EQ(router.route("GET", "/").handler.target_type(), typeid(root));
    EXPECT_EQ(router.route("GET", "/prefix/10000/nhope/aaaaa").handler.target_type(), typeid(a));
    EXPECT_EQ(router.route("GET", "/prefix/xxxxx/yyyyyyy/bbbbbb").handler.target_type(), typeid(b));
    EXPECT_EQ(router.route("GET", "/prefix/ccccc/bbbbbb").handler.target_type(), typeid(c));
}

TEST(Router, ExtractParamsFromPath)   // NOLINT
{
    Router router;
    router.get("/prefix/:a1/path2/:a2/aaaaa", [](RequestContext& /*ctx*/) {
        return nhope::makeReadyFuture();
    });
    router.get("/prefix/:b1/:b2/bbbbbb", [](RequestContext& /*ctx*/) {
        return nhope::makeReadyFuture();
    });
    router.get("/prefix/:c1/:c2/cccc/:c3", [](RequestContext& /*ctx*/) {
        return nhope::makeReadyFuture();
    });
    router.get("/:d1/:d2/:d3/:d4", [](RequestContext& /*ctx*/) {
        return nhope::makeReadyFuture();
    });

    {
        const auto etalon = RawPathParams{{"a1", "1000"}, {"a2", "2000"}};

        const auto r = router.route("GET", "/prefix/1000/path2/2000/aaaaa");
        EXPECT_EQ(r.rawPathParams, etalon);
    }

    {
        const auto etalon = RawPathParams{{"b1", "3000"}, {"b2", "4000"}};

        const auto r = router.route("GET", "/prefix/3000/4000/bbbbbb");
        EXPECT_EQ(r.rawPathParams, etalon);
    }

    {
        const auto etalon = RawPathParams{{"c1", "5000"}, {"c2", "6000"}, {"c3", "7000"}};

        const auto r = router.route("GET", "/prefix/5000/6000/cccc/7000");
        EXPECT_EQ(r.rawPathParams, etalon);
    }

    {
        const auto etalon = RawPathParams{{"d1", "8000"}, {"d2", "9000"}, {"d3", "10000"}, {"d4", "11000"}};

        const auto r = router.route("GET", "/8000/9000/10000/11000");
        EXPECT_EQ(r.rawPathParams, etalon);
    }
}

TEST(Router, NormalizePath)   //  NOLINT
{
    constexpr auto a = [](RequestContext& /*ctx*/) {
        return nhope::makeReadyFuture();
    };

    Router router;
    router.get("//a/b/..///c/d/", a);

    EXPECT_EQ(router.route("GET", "/a/c/d").handler.target_type(), typeid(a));
    EXPECT_EQ(router.route("GET", "a/../a//c/d").handler.target_type(), typeid(a));
}

TEST(Router, Use)   //  NOLINT
{
    constexpr auto root = [](RequestContext& /*ctx*/) {
        return nhope::makeReadyFuture();
    };
    constexpr auto subroot = [](RequestContext& /*ctx*/) {
        return nhope::makeReadyFuture();
    };
    constexpr auto a = [](RequestContext& /*ctx*/) {
        return nhope::makeReadyFuture();
    };
    constexpr auto b = [](RequestContext& /*ctx*/) {
        return nhope::makeReadyFuture();
    };
    constexpr auto c = [](RequestContext& /*ctx*/) {
        return nhope::makeReadyFuture();
    };
    constexpr auto d = [](RequestContext& /*ctx*/) {
        return nhope::makeReadyFuture();
    };
    constexpr auto e = [](RequestContext& /*ctx*/) {
        return nhope::makeReadyFuture();
    };
    constexpr auto f = [](RequestContext& /*ctx*/) {
        return nhope::makeReadyFuture();
    };

    Router subrouter;
    subrouter.get("/", subroot);
    subrouter.get("/a", a);
    subrouter.post("/a/b", b);
    subrouter.put("/a/b/:c", c);

    Router router;
    router.get("/", root);
    router.get("/a/b", d);
    router.get("/b", e);
    router.use("/b", std::move(subrouter));
    router.get("/b/f", f);

    EXPECT_EQ(router.route("GET", "/").handler.target_type(), typeid(root));
    EXPECT_EQ(router.route("GET", "/b").handler.target_type(), typeid(subroot));
    EXPECT_EQ(router.route("GET", "/b/a").handler.target_type(), typeid(a));
    EXPECT_EQ(router.route("POST", "/b/a/b").handler.target_type(), typeid(b));
    EXPECT_EQ(router.route("PUT", "/b/a/b/c:").handler.target_type(), typeid(c));
    EXPECT_EQ(router.route("GET", "/a/b").handler.target_type(), typeid(d));
    EXPECT_EQ(router.route("GET", "/b/f").handler.target_type(), typeid(f));
}

TEST(Router, NotFound)   // NOLINT
{
    constexpr auto subsubrouterNotFoundHandler = [](RequestContext& /*ctx*/) {
        return nhope::makeReadyFuture();
    };
    constexpr auto subrouterNotFoundHandler = [](RequestContext& /*ctx*/) {
        return nhope::makeReadyFuture();
    };
    const auto& defaultNotFoundHandler = Router().route("GET", "/not_exist_resource").handler;

    Router subsubrouter;
    subsubrouter.setNotFoundHandler(subsubrouterNotFoundHandler);

    Router subrouter;
    subrouter.setNotFoundHandler(subrouterNotFoundHandler);
    subrouter.use("/subsubrouter", std::move(subsubrouter));

    Router router;
    router.use("/subrouter", std::move(subrouter));
    EXPECT_EQ(router.route("GET", "/subrouter/subsubrouter/not_exist_resource").handler.target_type(),
              typeid(subsubrouterNotFoundHandler));
    EXPECT_EQ(router.route("GET", "/subrouter/not_exist_resource").handler.target_type(),
              typeid(subrouterNotFoundHandler));
    EXPECT_EQ(router.route("GET", "/not_exist_resource").handler.target_type(), defaultNotFoundHandler.target_type());
}

TEST(Router, MethodNotAllowed)   // NOLINT
{
    constexpr auto subsubrouterMethodNotAllowedHandler = [](RequestContext& /*ctx*/) {
        return nhope::makeReadyFuture();
    };
    constexpr auto subrouterMethodNotAllowedHandler = [](RequestContext& /*ctx*/) {
        return nhope::makeReadyFuture();
    };
    const auto& defaultMethodNotAllowedHandler = Router().route("METHOD_NOT_ALLOWED", "/").handler;

    Router subsubrouter;
    subsubrouter.setMethodNotAllowedHandler(subsubrouterMethodNotAllowedHandler);
    subsubrouter.get("/path", [](RequestContext& /*ctx*/) {
        return nhope::makeReadyFuture();
    });

    Router subrouter;
    subrouter.setMethodNotAllowedHandler(subrouterMethodNotAllowedHandler);
    subrouter.use("/subsubrouter", std::move(subsubrouter));
    subrouter.get("/path", [](RequestContext& /*ctx*/) {
        return nhope::makeReadyFuture();
    });

    Router router;
    router.use("/subrouter", std::move(subrouter));
    EXPECT_EQ(router.route("METHOD_NOT_ALLOWED", "/subrouter/subsubrouter/path").handler.target_type(),
              typeid(subsubrouterMethodNotAllowedHandler));
    EXPECT_EQ(router.route("METHOD_NOT_ALLOWED", "/subrouter/path").handler.target_type(),
              typeid(subrouterMethodNotAllowedHandler));
    EXPECT_EQ(router.route("METHOD_NOT_ALLOWED", "/").handler.target_type(),
              defaultMethodNotAllowedHandler.target_type());
}

TEST(Router, Middleware)   // NOLINT
{
    constexpr auto typeids = [](const auto& v) {
        std::vector<const std::type_info*> res;
        for (const auto& f : v) {
            res.push_back(&f.target_type());
        }
        return res;
    };

    constexpr auto subsubrouterMiddleware = [](RequestContext& /*ctx*/) {
        return nhope::makeReadyFuture<bool>(true);
    };
    constexpr auto subsubrouterMiddleware2 = [](RequestContext& /*ctx*/) {
        return nhope::makeReadyFuture<bool>(true);
    };

    constexpr auto subrouterMiddleware = [](RequestContext& /*ctx*/) {
        return nhope::makeReadyFuture<bool>(true);
    };

    Router subsubrouter;
    subsubrouter.addMiddleware(subsubrouterMiddleware);
    subsubrouter.addMiddleware(subsubrouterMiddleware2);
    subsubrouter.get("/resource", [](RequestContext& /*ctx*/) {
        return nhope::makeReadyFuture();
    });

    Router subrouter;
    subrouter.addMiddleware(subrouterMiddleware);
    subrouter.use("/subsubrouter", std::move(subsubrouter));
    subrouter.get("/resource", [](RequestContext& /*ctx*/) {
        return nhope::makeReadyFuture();
    });

    Router router;
    router.use("/subrouter", std::move(subrouter));

    {
        const auto etalon = std::vector{
          &typeid(subrouterMiddleware),
          &typeid(subsubrouterMiddleware),
          &typeid(subsubrouterMiddleware2),
        };
        const auto middlewares = router.route("GET", "/subrouter/subsubrouter/resource").middlewares;
        EXPECT_EQ(typeids(middlewares), etalon);
    }

    {
        const auto etalon = std::vector{
          &typeid(subrouterMiddleware),
        };
        const auto middlewares = router.route("GET", "/subrouter/resource").middlewares;
        EXPECT_EQ(typeids(middlewares), etalon);
    }
    {
        const auto middlewares = router.route("GET", "/resource").middlewares;
        EXPECT_TRUE(middlewares.empty());
    }
}

TEST(Router, AllowMethods)   // NOLINT
{
    constexpr auto handler = [](RequestContext& /*ctx*/) {
        return nhope::makeReadyFuture();
    };

    constexpr auto toSet = [](const auto& v) {
        return std::set<std::string>{v.begin(), v.end()};
    };

    Router router;
    router
      .get("/path1", handler)   //
      .post("/path1", handler)
      .put("/path1", handler)
      .patch("/path1/path2", handler)
      .options("/path1/path2", handler)
      .head("/path1/path2/path3", handler);

    EXPECT_EQ(toSet(router.allowMethods("/path1")), std::set({"GET"s, "POST"s, "PUT"s}));
    EXPECT_EQ(toSet(router.allowMethods("/path1/path2")), std::set({"PATCH"s, "OPTIONS"s}));
    EXPECT_EQ(toSet(router.allowMethods("/path1/path2/path3")), std::set({"HEAD"s}));
    EXPECT_EQ(toSet(router.allowMethods("/path1/path2/path3/path4")), std::set<std::string>());
}

TEST(Router, HightLevelHandler)   // NOLINT
{
    Router router;

    using intP = Param<int, "val">;
    router.get("/some/:val", [](const intP& p) {
        return p.get() + 2;
    });

    router.get("/some/", [] {
        return "42";
    });

    router.del("/some/", [] {
        return 1;
    });

    nhope::ThreadExecutor th;

    RequestContext ctx{
      .num = 1,
      .router = router,
      .rawPathParams = {{"val", "40"}},
      .aoCtx = nhope::AOContext(th),
    };
    {
        router.route("GET", "/some/40").handler(ctx).get();
        const auto body = nhope::readAll(*ctx.responce.body).get();
        const auto json = nlohmann::json::parse(body.begin(), body.end());
        EXPECT_EQ(json.get<int>(), 42);
    }
    {
        router.route("GET", "/some").handler(ctx).get();
        const auto body = nhope::readAll(*ctx.responce.body).get();
        const auto json = nlohmann::json::parse(body.begin(), body.end());
        EXPECT_EQ(json.get<std::string>(), "42");
    }

    {
        router.route("DELETE", "/some").handler(ctx).get();
        const auto body = nhope::readAll(*ctx.responce.body).get();
        const auto json = nlohmann::json::parse(body.begin(), body.end());
        EXPECT_EQ(json.get<int>(), 1);
    }
}

TEST(Router, BodyHandler)   // NOLINT
{
    Router router;

    constexpr bool x = royalbed::common::isBody<std::decay_t<const royalbed::common::Body<int>&>>;

    router.get("/some/", [](const royalbed::common::Body<int>& body) {
        return body.get() + 2;
    });

    nhope::ThreadExecutor th;
    nhope::AOContext ao(th);
    Request req;
    req.body = nhope::StringReader::create(ao, "2");
    req.headers.emplace("Content-type", "application/json");

    RequestContext ctx{
      .num = 1,
      .router = router,
      .request = std::move(req),
      .aoCtx = nhope::AOContext(th),
    };
    router.route("GET", "/some").handler(ctx).get();
    const auto body = nhope::readAll(*ctx.responce.body).get();
    const auto json = nlohmann::json::parse(body.begin(), body.end());
    EXPECT_EQ(json.get<int>(), 4);
}
