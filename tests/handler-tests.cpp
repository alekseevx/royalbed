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

std::unique_ptr<TestService> makeTestSrvFor(const std::string& path, const std::string& method,
                                            const std::function<LowLevelHandler>& handler)
{
    using Resources = TestService::Resources;

    const auto resource = std::make_shared<restbed::Resource>();
    resource->set_path(path);
    resource->set_method_handler(method, handler);

    return std::make_unique<TestService>(Resources{resource});
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

        return "responseData";
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
    const auto srv = makeTestSrvFor("/test", "GET", handler);

    const auto req = makeReq("/test", "GET");
    const auto resp = restbed::Http::sync(req);
    EXPECT_EQ(resp->get_status_code(), restbed::NOT_IMPLEMENTED);
}
