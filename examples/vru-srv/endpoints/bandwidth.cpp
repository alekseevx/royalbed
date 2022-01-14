
#include "nlohmann/json.hpp"
#include "royalbed/server/body.h"

#include "../vru/storage.h"

#include "params.h"
#include "bandwidth.h"

namespace vru_srv::endpoints {
namespace {
using royalbed::server::Body;

struct BandWidth
{
    int bandWidth{};
};

void from_json(const nlohmann::json& jsonValue, BandWidth& value)
{
    jsonValue.at("bandWidth").get_to(value.bandWidth);
}

void to_json(nlohmann::json& jsonValue, const BandWidth& value)
{
    jsonValue = nlohmann::json{
      {"bandWidth", value.bandWidth},
    };
}

}   // namespace

void publicBandwidthEndpoint(royalbed::server::Router& router)
{
    router.get(":vruId/bandwidth", [](const VruId& vruId) {
        const auto& storage = vru::Storage::instance();
        return storage.getBandWidth(*vruId);
    });

    router.put(":vruId/bandwidth", [](const VruId& vruId, const Body<BandWidth>& body) {
        auto& storage = vru::Storage::instance();
        storage.setBandWidth(*vruId, body->bandWidth);
    });
}

}   // namespace vru_srv::endpoints
