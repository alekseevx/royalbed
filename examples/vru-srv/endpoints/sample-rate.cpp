#include "nlohmann/json.hpp"
#include "royalbed/server/body.h"

#include "../vru/storage.h"
#include "params.h"
#include "sample-rate.h"

namespace vru_srv {
namespace {
using royalbed::server::Body;

struct SampleRate
{
    int sampleRate{};
};

void from_json(const nlohmann::json& jsonValue, SampleRate& value)
{
    jsonValue.at("sampleRate").get_to(value.sampleRate);
}

void to_json(nlohmann::json& jsonValue, const SampleRate& value)
{
    jsonValue = nlohmann::json{
      {"sampleRate", value.sampleRate},
    };
}

}   // namespace

void publicSampleRateEndpoint(royalbed::server::Router& router)
{
    router.get(":vruId/sample-rate", [](const VruId& vruId) {
        const auto& storage = vru::Storage::instance();
        const auto samplingRate = storage.getSampleRate(*vruId);
        return SampleRate{samplingRate};
    });

    router.put(":vruId/sample-rate", [](const VruId& vruId, const Body<SampleRate>& body) {
        auto& storage = vru::Storage::instance();
        const auto samplingRate = body->sampleRate;
        storage.setSampleRate(*vruId, samplingRate);
    });
}

}   // namespace vru_srv
