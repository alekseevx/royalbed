#pragma once

#include <string>

#include "fsp.h"
#include "status.h"

namespace vru_srv::vru {

struct State
{
    std::string id;
    Status status = Status::NotActive;
    int sampleRate = 8000;
    int hfChannelNum = 0;
    FspMap prearrangedFreqList;
    short prearrangedFreqNum = -1;
    long long freq = 1500000;
    int bandWidth = 150;
};
using StateMap = std::map<std::string, State>;

void to_json(nlohmann::json& jsonValue, const State& state);
void to_json(nlohmann::json& jsonValue, StateMap& stateMap);

}   // namespace vru_srv::vru
