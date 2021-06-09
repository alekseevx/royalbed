#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

#include <gtest/gtest.h>

#include <corvusoft/restbed/http.hpp>
#include <corvusoft/restbed/settings.hpp>
#include <corvusoft/restbed/resource.hpp>
#include <corvusoft/restbed/status_code.hpp>
#include <nlohmann/json.hpp>

#include <royalbed/handler.h>
#include <royalbed/http-error.h>
#include <royalbed/param.h>

#include "helpers/resp.h"
#include "helpers/req.h"
#include "helpers/test-service.h"

namespace {
using namespace std::literals;
using namespace royalbed;
using namespace nlohmann;

std::shared_ptr<restbed::Resource> makeResource(const std::string& path, const std::string& method,
                                                const std::function<LowLevelHandler>& handler)
{
    auto resource = std::make_shared<restbed::Resource>();
    resource->set_path(path);
    resource->set_method_handler(method, handler);
    return resource;
}

std::unique_ptr<TestService> makeTestSrvFor(const std::string& path, const std::string& method,
                                            const std::function<LowLevelHandler>& handler)
{
    using Resources = TestService::Resources;
    return std::make_unique<TestService>(Resources{makeResource(path, method, handler)});
}

}   // namespace

TEST(Handler, WithoutParams)   // NOLINT
{
    const auto handler = makeLowLevelHandler(restbed::OK, [] {});
    const auto srv = makeTestSrvFor("/test", "POST", handler);

    const auto req = makeReq("/test", "POST");
    const auto resp = restbed::Http::sync(req);
    EXPECT_EQ(resp->get_status_code(), restbed::OK);
}

TEST(Handler, ParamsRequestBodyEmptyResponce)   // NOLINT
{
    static constexpr auto idParamName = "id"sv;
    using Id = PathParam<std::uint64_t, idParamName, Required>;

    static constexpr auto statusParamName = "status"sv;
    using Status = QueryParam<std::string, statusParamName>;

    using Body = Body<std::string>;

    const auto handler = makeLowLevelHandler(restbed::OK, [](const Id& id, const Status& status, const Body& body) {
        EXPECT_EQ(id.get(), 1000);
        EXPECT_EQ(status.get(), "active");
        EXPECT_EQ(body.get(), "data");
    });
    const auto srv = makeTestSrvFor("/test/{id: .*}", "GET", handler);

    const auto req = makeReq("/test/1000?status=active", "GET", json("data"));
    const auto resp = restbed::Http::sync(req);
    EXPECT_EQ(resp->get_status_code(), restbed::OK);
    EXPECT_EQ(resp->get_body().size(), 0);
}

TEST(Handler, ParamsRequestBodyResponce)   // NOLINT
{
    static constexpr auto idParamName = "id"sv;
    using Id = PathParam<std::uint64_t, idParamName, Required>;

    static constexpr auto statusParamName = "status"sv;
    using Status = QueryParam<std::string, statusParamName>;

    using Body = Body<std::string>;

    const auto handler = makeLowLevelHandler(restbed::OK, [](const Id& id, const Status& status, const Body& body) {
        EXPECT_EQ(id.get(), 1000);
        EXPECT_EQ(status.get(), "active");
        EXPECT_EQ(body.get(), "data");

        return "responseData"s;
    });
    const auto srv = makeTestSrvFor("/test/{id: .*}", "GET", handler);

    const auto req = makeReq("/test/1000?status=active", "GET", json("data"));
    const auto resp = restbed::Http::sync(req);
    EXPECT_EQ(resp->get_status_code(), restbed::OK);

    const auto respBody = fetchBody(resp);
    EXPECT_EQ(respBody, json("responseData"));
}

TEST(Handler, BadParam)   // NOLINT
{
    static constexpr auto idParamName = "id"sv;
    using Id = PathParam<std::uint64_t, idParamName, Required>;

    const auto handler = makeLowLevelHandler(restbed::OK, [](const Id& /*id*/) {
        ADD_FAILURE();
    });
    const auto srv = makeTestSrvFor("/test/{id: .*}", "GET", handler);

    const auto req = makeReq("/test/bad-id", "GET");
    const auto resp = restbed::Http::sync(req);
    EXPECT_EQ(resp->get_status_code(), restbed::BAD_REQUEST);
}

TEST(Handler, Exception)   // NOLINT
{
    const auto handler = makeLowLevelHandler(restbed::OK, [] {
        throw HttpError(restbed::NOT_IMPLEMENTED, "Not implemented");
    });
    const auto badHandler = makeLowLevelHandler(restbed::OK, [] {
        throw std::runtime_error("something bad");
    });

    const auto veryBadHandler = makeLowLevelHandler(restbed::OK, [] {
        throw 1;
    });

    const auto path = "/test"s;
    const auto badPath = "/bad"s;
    const auto veryBadPath = "/verybad"s;

    TestService::Resources resources;
    resources.emplace_back(makeResource(path, "GET", handler));
    resources.emplace_back(makeResource(badPath, "GET", badHandler));
    resources.emplace_back(makeResource(veryBadPath, "GET", veryBadHandler));
    const auto srv = std::make_unique<TestService>(resources);

    const auto req = makeReq(path, "GET");
    const auto resp = restbed::Http::sync(req);
    EXPECT_EQ(resp->get_status_code(), restbed::NOT_IMPLEMENTED);

    const auto req2 = makeReq(badPath, "GET");
    const auto resp2 = restbed::Http::sync(req2);
    EXPECT_EQ(resp2->get_status_code(), restbed::INTERNAL_SERVER_ERROR);

    const auto req3 = makeReq(veryBadPath, "GET");
    const auto resp3 = restbed::Http::sync(req3);
    EXPECT_EQ(resp3->get_status_code(), restbed::INTERNAL_SERVER_ERROR);
}
