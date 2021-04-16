#include <corvusoft/restbed/http.hpp>
#include <corvusoft/restbed/settings.hpp>

#include "nlohmann/json.hpp"
#include "resp.h"

nlohmann::json fetchBody(const std::shared_ptr<restbed::Response>& resp)
{
    const auto contentLength = resp->get_header("Content-Length", std::size_t(0));
    restbed::Http::fetch(contentLength, resp);
    const auto rawBody = resp->get_body();
    return nlohmann::json::parse(rawBody);
}
