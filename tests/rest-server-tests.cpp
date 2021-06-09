#include <string>
#include <thread>
#include <gtest/gtest.h>

#include <royalbed/rest-server.h>
#include <royalbed/router.h>

#include <corvusoft/restbed/http.hpp>
#include <corvusoft/restbed/settings.hpp>
#include <corvusoft/restbed/status_code.hpp>

#include "helpers/resp.h"
#include "helpers/req.h"
#include "helpers/test-service.h"
#include "royalbed/body.h"

namespace {
using namespace std::literals;
using namespace royalbed;
}   // namespace

TEST(RestServer, Create)   // NOLINT
{
    auto router = Router();
    router.get("/test", [] {
        return "Test"s;
    });

    router.put("/test", [](const Body<std::string>& body) {
        EXPECT_EQ(body.get(), "TestData");
    });

    auto srv = RestServer();
    srv.setBindAddress("127.0.0.1")
      .setPort(TestService::httpPort)
      .setThreadCount(1)
      .setReuseAddress(true)
      .setKeepAlive(true)
      .setConnectionTimeout(5s)
      .setRouter(router);

    auto srvThread = std::thread([&srv] {
        ASSERT_NO_THROW({ srv.listen(); });   // NOLINT
    });

    // Wait for call srv.listen()
    std::this_thread::sleep_for(250ms);

    {
        const auto req = makeReq("/test", "GET");
        const auto resp = restbed::Http::sync(req);
        EXPECT_EQ(resp->get_status_code(), restbed::OK);
        const auto respBody = fetchBody(resp);
        EXPECT_EQ(respBody, nlohmann::json("Test"));
    }

    {
        const auto req = makeReq("/test", "PUT", nlohmann::json("TestData"));
        const auto resp = restbed::Http::sync(req);
        EXPECT_EQ(resp->get_status_code(), restbed::OK);
    }

    srv.stop();
    srvThread.join();
}
