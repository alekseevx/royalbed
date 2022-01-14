#pragma once

#include <map>
#include <tuple>

#include "nlohmann/json.hpp"

namespace vru_srv::vru {

/**
 * Заранее подготовленная частота (ЗПЧ).
 */
struct Fsp
{
    /**
     * < Глобальный номер частоты на тракте (в УМ).
     */
    short globalNumberOnTract;
    /**
     * < Номенал частоты.
     */
    long long int freq;
    /**
     * < Тип антенны.
     */
    short antenna;
    /**
     * < Настроенна ли данная частота.
     */
    bool isApply;

    /**
     * Obtains a tuple containing all of the struct's data members.
     * @return The data members in a tuple.
     */
    [[nodiscard]] std::tuple<const short&, const long long int&, const short&, const bool&> ice_tuple() const
    {
        return std::tie(globalNumberOnTract, freq, antenna, isApply);
    }
};

using FspMap = std::map<short, Fsp>;

void from_json(const nlohmann::json& jsonValue, Fsp& fsp);
void from_json(const nlohmann::json& jsonValue, FspMap& fspMap);

}   // namespace vru_srv::vru
