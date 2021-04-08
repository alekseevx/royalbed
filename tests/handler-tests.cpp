#include <cstdint>
#include <functional>
#include <memory>
#include <string_view>
#include <string>
#include <thread>

#include <corvusoft/restbed/http.hpp>
#include <corvusoft/restbed/request.hpp>
#include <corvusoft/restbed/resource.hpp>
#include <corvusoft/restbed/response.hpp>
#include <corvusoft/restbed/service.hpp>
#include <corvusoft/restbed/settings.hpp>
#include <corvusoft/restbed/status_code.hpp>
#include <corvusoft/restbed/uri.hpp>

#include <gtest/gtest.h>

#include <nlohmann/json.hpp>
#include <royalbed/handler.h>
#include <royalbed/param.h>

#include "helpers.h"
#include "royalbed/http-error.h"

namespace {
using namespace std::literals;
using namespace royalbed;

void publish(restbed::Service& srv, const std::string& path, const std::string& method,
             const std::function<LowLevelHandler>& handler)
{
    auto resource = std::make_shared<restbed::Resource>();
    resource->set_path(path);
    resource->set_method_handler(method, handler);
    srv.publish(resource);
}

}   // namespace

TEST(Handler, WithoutParams)   // NOLINT
{
    auto handler = makeLowLevelHandler(restbed::OK, [] {});
    restbed::Service srv;
    publish(srv, "/test", "POST", handler);
    start(srv);

    auto req = makeRequest("/test", "POST");
    auto resp = restbed::Http::sync(req);
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
    restbed::Service srv;
    publish(srv, "/test/{id: .*}", "GET", handler);
    start(srv);

    auto req = makeRequest("/test/1000?status=active", "GET");
    req->add_header("Content-Type", "application/json");
    setBody(req, "\"data\"");
    auto resp = restbed::Http::sync(req);
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
    restbed::Service srv;
    publish(srv, "/test/{id: .*}", "GET", handler);
    start(srv);

    auto req = makeRequest("/test/1000?status=active", "GET");
    req->add_header("Content-Type", "application/json");
    setBody(req, "\"data\"");
    auto resp = restbed::Http::sync(req);

    EXPECT_EQ(resp->get_status_code(), restbed::OK);

    const auto respBody = fetchBody(resp);
    const auto respData = nlohmann::json::parse(respBody);
    EXPECT_EQ(respData, nlohmann::json("responseData"));
}

TEST(Handler, BadParam)   // NOLINT
{
    static constexpr auto idParamName = "id"sv;
    using Id = PathParam<std::uint64_t, idParamName, Required>;

    const auto handler = makeLowLevelHandler(restbed::OK, [](const Id& id) {
        ADD_FAILURE();
    });
    restbed::Service srv;
    publish(srv, "/test/{id: .*}", "GET", handler);
    start(srv);

    auto req = makeRequest("/test/bad-id", "GET");
    auto resp = restbed::Http::sync(req);
    EXPECT_EQ(resp->get_status_code(), restbed::BAD_REQUEST);
}

TEST(Handler, Exception)   // NOLINT
{
    const auto handler = makeLowLevelHandler(restbed::OK, [] {
        throw HttpError(restbed::NOT_IMPLEMENTED, "Not implemented");
    });
    restbed::Service srv;
    publish(srv, "/test", "GET", handler);
    start(srv);

    auto req = makeRequest("/test", "GET");
    auto resp = restbed::Http::sync(req);
    EXPECT_EQ(resp->get_status_code(), restbed::NOT_IMPLEMENTED);
}
