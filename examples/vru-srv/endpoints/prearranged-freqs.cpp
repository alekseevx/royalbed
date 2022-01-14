#include <cstddef>

#include "nlohmann/json.hpp"
#include "royalbed/server/body.h"

#include "../vru/storage.h"

#include "params.h"
#include "prearranged-freqs.h"

namespace vru_srv {
namespace {
using royalbed::server::Body;

struct PutPrearrangedFreqsParams
{
    vru::FspMap prearrangedFreqList;
    short hfChannelNum;
};

void from_json(const nlohmann::json& jsonValue, PutPrearrangedFreqsParams& value)
{
    jsonValue["prearrangedFreqList"].get_to(value.prearrangedFreqList);
    if (jsonValue.contains("hfChannelNum")) {
        value.hfChannelNum = short(jsonValue["hfChannelNum"]);
    }
}

void publicPrearrangedFreqsEndpoint(royalbed::server::Router& router)
{
    router.put(":vruId/prearranged-freqs",
               [](const VruId& vruId, const royalbed::common::Body<PutPrearrangedFreqsParams>& body) {
                   auto& storage = vru::Storage::instance();
                   storage.setPrearrangedFreqs(*vruId, body->prearrangedFreqList);
                   storage.setHfChannelNum(*vruId, body->hfChannelNum);
               });
}

}   // namespace

void publicPrearrangedFreqsEndpoints(royalbed::server::Router& router)
{
    publicPrearrangedFreqsEndpoint(router);
}

}   // namespace vru_srv
