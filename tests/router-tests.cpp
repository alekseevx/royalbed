#include <string>
#include <string_view>

#include <gtest/gtest.h>

#include <royalbed/router.h>
#include <corvusoft/restbed/service.hpp>
#include <corvusoft/restbed/status_code.hpp>
#include <vector>

#include "helpers.h"
#include "nlohmann/json.hpp"

namespace {
using namespace std::literals;
using namespace royalbed;

}   // namespace

TEST(Router, addRotes)   // NOLINT
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

    restbed::Service srv;
    for (auto& resource : router.resources()) {
        srv.publish(resource);
    }
    start(srv);

    struct TestRec
    {
        std::string path;
        std::string method;
        int responseStatus = restbed::OK;
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
        auto req = makeRequest(rec.path, rec.method);
        auto resp = restbed::Http::sync(req);
        EXPECT_EQ(resp->get_status_code(), rec.responseStatus);

        const auto respBody = fetchBody(resp);
        const auto respData = nlohmann::json::parse(respBody);
        EXPECT_EQ(respData, nlohmann::json(fmt::format("{} {}", rec.method, rec.path)));
    }
}

TEST(Router, addSubRotes)   // NOLINT
{
    Router router;
    {
        auto subRouter = router.route("/path");
        subRouter.get("/path2", [] {});
    }

    restbed::Service srv;
    for (auto& resource : router.resources()) {
        srv.publish(resource);
    }

    start(srv);
    auto req = makeRequest("/path/path2", "GET");
    auto resp = restbed::Http::sync(req);
    EXPECT_EQ(resp->get_status_code(), restbed::OK);
}
