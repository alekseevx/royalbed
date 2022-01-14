#include "nlohmann/json.hpp"

#include "../vru/storage.h"

#include "params.h"
#include "status.h"

namespace vru_srv {

namespace {

struct VruStatus
{
    std::string status;
};

void to_json(nlohmann::json& jsonValue, const VruStatus& value)
{
    jsonValue = nlohmann::json{
      {"status", value.status},
    };
}

}   // namespace

void publicStatusEndpoint(royalbed::server::Router& router)
{
    router.get(":vruId/status", [](const VruId& vruId) {
        const auto& storage = vru::Storage::instance();
        const auto status = storage.status(*vruId);
        return VruStatus{
          .status = vru::toString(status),
        };
    });
}

}   // namespace vru_srv
