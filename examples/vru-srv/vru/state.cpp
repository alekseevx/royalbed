#include "state.h"
#include "status.h"

namespace vru_srv::vru {

void to_json(nlohmann::json& jsonValue, const State& state)
{
    jsonValue = nlohmann::json{
      {"id", state.id},   //
      {"status", toString(state.status)},
      {"emissionClass", "RAW"},
      {"sampleRate", state.sampleRate},
      {"hfChannelCount", 3},
    };
}

void to_json(nlohmann::json& jsonValue, StateMap& stateMap)
{
    for (const auto& [id, info] : stateMap) {
        nlohmann::json jsonInfo;
        to_json(jsonInfo, info);
        jsonValue.push_back(jsonInfo);
    };
}

}   // namespace vru_srv::vru
