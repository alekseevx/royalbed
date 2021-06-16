#include <memory>

#include <corvusoft/restbed/http.hpp>
#include <corvusoft/restbed/resource.hpp>
#include <corvusoft/restbed/session.hpp>
#include <corvusoft/restbed/settings.hpp>
#include <corvusoft/restbed/status_code.hpp>

#include <gtest/gtest.h>

#include <royalbed/detail/fetch-body-rule.h>
#include <string>

#include "corvusoft/restbed/rule.hpp"
#include "helpers/resp.h"
#include "helpers/req.h"
#include "helpers/test-service.h"

namespace {
using namespace royalbed;
using namespace royalbed::detail;
using namespace std::literals;

const auto sizeLimit = 10;
const auto etalonData = "1234567890"s;      // NOLINT(cert-err58-cpp)
const auto tooLargeData = "1234567890!"s;   // NOLINT(cert-err58-cpp)

std::unique_ptr<TestService> makeTestServer()   // NOLINT
{
    const auto fetchBodyRule = std::make_shared<FetchBodyRule>("application/fetch", sizeLimit);

    const auto resource = std::make_shared<restbed::Resource>();
    resource->set_path("/test");
    resource->set_method_handler("PUT", [](const std::shared_ptr<restbed::Session>& session) {
        const auto req = session->get_request();
        if (req->get_header("Content-Type") != "application/fetch") {
            EXPECT_TRUE(req->get_body().empty());
        } else {
            const auto contentLength = req->get_header("Content-Length", std::size_t(0));
            EXPECT_LE(contentLength, sizeLimit);
            EXPECT_EQ(req->get_body().size(), contentLength);
        }

        session->close(restbed::OK);
    });

    return std::make_unique<TestService>(TestService::Resources{resource}, TestService::Rules{fetchBodyRule});
}

}   // namespace

TEST(FetchBodyRule, NotFetch)   // NOLINT
{
    const auto srv = makeTestServer();

    const auto req = makeReq("/test", "PUT");
    req->set_body(etalonData);
    req->set_header("Content-Type", "application/other");
    req->set_header("Content-Length", std::to_string(etalonData.size()));
    const auto resp = restbed::Http::sync(req);
    EXPECT_EQ(resp->get_status_code(), restbed::OK);
}

TEST(FetchBodyRule, WrongContentLength)   // NOLINT
{
    const auto srv = makeTestServer();

    {
        const auto req = makeReq("/test", "PUT");
        req->set_body(etalonData);
        req->set_header("Content-Type", "application/fetch");
        const auto resp = restbed::Http::sync(req);
        EXPECT_EQ(resp->get_status_code(), restbed::LENGTH_REQUIRED);
    }

    {
        const auto req = makeReq("/test", "PUT");
        req->set_body(tooLargeData);
        req->set_header("Content-Type", "application/fetch");
        req->set_header("Content-Length", std::to_string(tooLargeData.size()));
        const auto resp = restbed::Http::sync(req);
        EXPECT_EQ(resp->get_status_code(), restbed::REQUEST_ENTITY_TOO_LARGE);
    }
}

TEST(FetchBodyRule, Fetch)   // NOLINT
{
    const auto srv = makeTestServer();

    const auto req = makeReq("/test", "PUT");
    req->set_body(tooLargeData);
    req->set_header("Content-Type", "application/fetch");
    req->set_header("Content-Length", std::to_string(etalonData.size()));
    const auto resp = restbed::Http::sync(req);
    EXPECT_EQ(resp->get_status_code(), restbed::OK);
}
