#include <variant>

#include "nlohmann/json.hpp"
#include "royalbed/server/body.h"

#include "../vru/storage.h"
#include "params.h"
#include "freq.h"

namespace vru_srv {

namespace {
using namespace std::literals;
using royalbed::server::Body;

struct FreqValue
{
    long long freq{};
};

struct PrearrangedFreq
{
    short prearrangedFreqNum{};
};

struct PutFreqParams
{
    std::variant<FreqValue, PrearrangedFreq> freq;
    short hfChannelNum;
};

void from_json(const nlohmann::json& jsonValue, PutFreqParams& value)
{
    if (jsonValue.contains("hfChannelNum")) {
        value.hfChannelNum = short(jsonValue["hfChannelNum"]);
    }

    if (jsonValue.contains("prearrangedFreqNum")) {
        value.freq = PrearrangedFreq{jsonValue["prearrangedFreqNum"]};
    } else {
        value.freq = FreqValue{jsonValue.at("freq")};
    }
}
}   // namespace

void publicFreqEndpoint(royalbed::server::Router& router)
{
    router.put(":vruId/freq", [](const VruId& vruId, const Body<PutFreqParams>& body) {
        auto& storage = vru::Storage::instance();
        if (const auto* freqValue = std::get_if<FreqValue>(&body->freq)) {
            storage.setFreq(*vruId, freqValue->freq, body->hfChannelNum);
        } else {
            auto prearrangedFreq = std::get<PrearrangedFreq>(body->freq);
            storage.useFreqForPrearranged(*vruId, prearrangedFreq.prearrangedFreqNum, body->hfChannelNum);
        }
    });
}

}   // namespace vru_srv
