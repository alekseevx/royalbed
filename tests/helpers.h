#pragma once

#include "corvusoft/restbed/byte.hpp"
#include "corvusoft/restbed/response.hpp"
#include <cstdint>
#include <memory>
#include <string>
#include <thread>

#include <fmt/format.h>

#include <corvusoft/restbed/http.hpp>
#include <corvusoft/restbed/request.hpp>
#include <corvusoft/restbed/service.hpp>
#include <corvusoft/restbed/settings.hpp>

#include <royalbed/rules/fetch-body-rule.h>

constexpr inline std::uint16_t httpPort = 5555;

inline std::shared_ptr<restbed::Request> makeRequest()
{
    auto req = std::make_shared<restbed::Request>();
    req->set_host("127.0.0.1");
    req->set_port(httpPort);
    return req;
}

inline std::shared_ptr<restbed::Request> makeRequest(const std::string& path, const std::string& method)
{
    auto req = makeRequest();
    req->set_path(path);
    req->set_method(method);
    return req;
}

inline void setBody(const std::shared_ptr<restbed::Request>& req, const std::string& body)
{
    req->set_body(body);
    req->set_header("Content-Length", std::to_string(body.size()));
}

inline restbed::Bytes fetchBody(const std::shared_ptr<restbed::Response>& resp)
{
    const auto contentLength = resp->get_header("Content-Length", std::size_t(0));
    restbed::Http::fetch(contentLength, resp);
    return resp->get_body();
}

inline void start(restbed::Service& srv)
{
    std::thread([&srv] {
        auto fetchBodyRule = std::make_shared<royalbed::FetchBodyRule>("application/json");
        srv.add_rule(fetchBodyRule);

        auto settings = std::make_shared<restbed::Settings>();
        settings->set_port(httpPort);
        settings->set_reuse_address(true);
        settings->set_bind_address("127.0.0.1");
        srv.start(settings);
    }).detach();
}
