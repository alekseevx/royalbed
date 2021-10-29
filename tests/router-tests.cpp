#include <string>
#include <vector>

#include <gtest/gtest.h>

#include <corvusoft/restbed/http.hpp>
#include <corvusoft/restbed/settings.hpp>
#include <nlohmann/json.hpp>

#include "royalbed/http-status.h"
#include "royalbed/router.h"

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
               return "GET /path"s;
           })
      .post("/path",
            [] {
                return "POST /path"s;
            })
      .put("/path",
           [] {
               return "PUT /path"s;
           })
      .del("/path",
           [] {
               return "DELETE /path"s;
           })
      .get("/path2", HttpStatus::ResetContent,
           [] {
               return "GET /path2"s;
           })
      .post("/path2", HttpStatus::ResetContent,
            [] {
                return "POST /path2"s;
            })
      .put("/path2", HttpStatus::ResetContent,
           [] {
               return "PUT /path2"s;
           })
      .del("/path2", HttpStatus::ResetContent, [] {
          return "DELETE /path2"s;
      });

    EXPECT_EQ(router.resources().size(), 2);

    const auto srv = TestService(router.resources());

    struct TestRec
    {
        const std::string path;
        const std::string method;
        const int responseStatus = HttpStatus::Ok;
    };
    const auto testRecs = std::vector<TestRec>{
      {"/path", "GET"},
      {"/path", "POST", HttpStatus::Created},   // Default status for POST is 201 (HttpStatus::Created)
      {"/path", "PUT"},
      {"/path", "DELETE"},
      {"/path2", "GET", HttpStatus::ResetContent},
      {"/path2", "POST", HttpStatus::ResetContent},
      {"/path2", "PUT", HttpStatus::ResetContent},
      {"/path2", "DELETE", HttpStatus::ResetContent},
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
    Router router("/prefix");
    {
        Router subRouter;
        subRouter.get("/path", [] {});
        router.use("/subroutes", subRouter);
    }

    const auto srv = TestService(router.resources());

    const auto req = makeReq("/prefix/subroutes/path", "GET");
    const auto resp = restbed::Http::sync(req);
    EXPECT_EQ(resp->get_status_code(), HttpStatus::Ok);
}
