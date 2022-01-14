#include "fsp.h"

namespace vru_srv::vru {

void from_json(const nlohmann::json& jsonValue, Fsp& fsp)
{
    jsonValue.at("globalNumberOnTract").get_to(fsp.globalNumberOnTract);
    jsonValue.at("freq").get_to(fsp.freq);
    jsonValue.at("antennaNum").get_to(fsp.antenna);
    if (jsonValue.contains("prepared")) {
        jsonValue.at("prepared").get_to(fsp.isApply);
    } else {
        fsp.isApply = false;
    }
}

void from_json(const nlohmann::json& jsonValue, FspMap& fspMap)
{
    for (std::size_t i = 0; i < jsonValue.size(); ++i) {
        jsonValue[i].get_to(fspMap[static_cast<short>(i)]);
    }
}

}   // namespace vru_srv::vru
