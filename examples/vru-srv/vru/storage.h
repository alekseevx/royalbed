#pragma once

#include <cstddef>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <string_view>

#include "state.h"
#include "status.h"

namespace vru_srv::vru {

class Storage
{
    Storage();

public:
    Storage(Storage&& /*unused*/) noexcept = delete;
    Storage(const Storage& /*unused*/) noexcept = delete;

    static Storage& instance();

    [[nodiscard]] StateMap getAllVru() const;

    void activate(std::string_view idVru);
    void deactivate(std::string_view idVru);
    [[nodiscard]] Status status(std::string_view idVru) const;

    void setSampleRate(std::string_view idVru, int value);
    [[nodiscard]] int getSampleRate(std::string_view idVru) const;

    void setPrearrangedFreqs(std::string_view idVru, FspMap value);
    void setHfChannelNum(std::string_view idVru, int value);

    void setFreq(std::string_view idVru, long long freq, int hfChannelNum);
    void useFreqForPrearranged(std::string_view idVru, short prearrangedFreqNum, int hfChannelNum);

    void setBandWidth(std::string_view idVru, int value);
    [[nodiscard]] int getBandWidth(std::string_view idVru) const;

private:
    StateMap m_storage;
};

}   // namespace vru_srv::vru
