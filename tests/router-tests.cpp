#include <string>
#include <vector>

#include <gtest/gtest.h>

#include <corvusoft/restbed/http.hpp>
#include <corvusoft/restbed/settings.hpp>
#include <corvusoft/restbed/status_code.hpp>
#include <nlohmann/json.hpp>

#include <royalbed/router.h>

#include "helpers/resp.h"
#include "helpers/req.h"
#include "helpers/test-service.h"

namespace {
using namespace std::literals;
using namespace royalbed;
}   // namespace

TEST(Router, addRoutes)   // NOLINT
{
    Router router;

    router
      .get("path",
           [] {
               return "GET /path";
           })
      .post("/path",
            [] {
                return "POST /path";
            })
      .put("/path",
           [] {
               return "PUT /path";
           })
      .del("/path",
           [] {
               return "DELETE /path";
           })
      .get("/path2", restbed::RESET_CONTENT,
           [] {
               return "GET /path2";
           })
      .post("/path2", restbed::RESET_CONTENT,
            [] {
                return "POST /path2";
            })
      .put("/path2", restbed::RESET_CONTENT,
           [] {
               return "PUT /path2";
           })
      .del("/path2", restbed::RESET_CONTENT, [] {
          return "DELETE /path2";
      });

    EXPECT_EQ(router.resources().size(), 2);

    const auto srv = TestService(router.resources());

    struct TestRec
    {
        const std::string path;
        const std::string method;
        const int responseStatus = restbed::OK;
    };
    const auto testRecs = std::vector<TestRec>{
      {"/path", "GET"},
      {"/path", "POST", restbed::CREATED},   // Default status for POST is 201 (restbed::CREATED)
      {"/path", "PUT"},
      {"/path", "DELETE"},
      {"/path2", "GET", restbed::RESET_CONTENT},
      {"/path2", "POST", restbed::RESET_CONTENT},
      {"/path2", "PUT", restbed::RESET_CONTENT},
      {"/path2", "DELETE", restbed::RESET_CONTENT},
    };

    for (const auto& rec : testRecs) {
        const auto req = makeReq(rec.path, rec.method);
        const auto resp = restbed::Http::sync(req);
        EXPECT_EQ(resp->get_status_code(), rec.responseStatus);

        const auto respBody = fetchBody(resp);
        EXPECT_EQ(respBody, nlohmann::json(fmt::format("{} {}", rec.method, rec.path)));
    }
}

TEST(Router, addSubRoutes)   // NOLINT
{
    Router router;
    {
        Router subRouter;
        subRouter.get("/path", [] {});
        router.use("/subroutes", subRouter);
    }

    const auto srv = TestService(router.resources());

    const auto req = makeReq("/subroutes/path", "GET");
    const auto resp = restbed::Http::sync(req);
    EXPECT_EQ(resp->get_status_code(), restbed::OK);
}
