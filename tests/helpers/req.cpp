#include "test-service.h"
#include "req.h"

std::shared_ptr<restbed::Request> makeReq()
{
    auto req = std::make_shared<restbed::Request>();
    req->set_host("127.0.0.1");
    req->set_port(TestService::httpPort);
    return req;
}

std::shared_ptr<restbed::Request> makeReq(const std::string& path, const std::string& method)
{
    auto req = makeReq();
    req->set_path(path);
    req->set_method(method);
    return req;
}

std::shared_ptr<restbed::Request> makeReq(const std::string& path, const std::string& method,
                                          const nlohmann::json& body)
{
    auto req = makeReq(path, method);

    const auto rawBody = nlohmann::to_string(body);
    req->set_body(rawBody);
    req->set_header("Content-Type", "application/json");
    req->set_header("Content-Length", std::to_string(rawBody.size()));

    return req;
}
