#pragma once

#include <memory>
#include <string>

#include <corvusoft/restbed/request.hpp>
#include <nlohmann/json.hpp>

std::shared_ptr<restbed::Request> makeReq();
std::shared_ptr<restbed::Request> makeReq(const std::string& path, const std::string& method);
std::shared_ptr<restbed::Request> makeReq(const std::string& path, const std::string& method,
                                          const nlohmann::json& body);
