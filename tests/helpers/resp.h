#pragma once

#include <memory>
#include <corvusoft/restbed/response.hpp>
#include <nlohmann/json.hpp>

nlohmann::json fetchBody(const std::shared_ptr<restbed::Response>& resp);
