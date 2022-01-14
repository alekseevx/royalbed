#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "fmt/core.h"

#include "status.h"
#include "storage.h"

namespace vru_srv::vru {

Storage::Storage()
{
    constexpr auto countVru = 10;
    for (int i = 0; i < countVru; ++i) {
        std::string id = fmt::format("vru_{}", i);
        m_storage.insert({id, State({.id = id})});
    }
}

Storage& Storage::instance()
{
    static Storage inst;
    return inst;
}

StateMap Storage::getAllVru() const
{
    return m_storage;
}

void Storage::activate(std::string_view idVru)
{
    m_storage.at(std::string(idVru)).status = Status::Active;
}

void Storage::deactivate(std::string_view idVru)
{
    m_storage.at(std::string(idVru)).status = Status::NotActive;
}

Status Storage::status(std::string_view idVru) const
{
    return m_storage.at(std::string(idVru)).status;
}

void Storage::setSampleRate(std::string_view idVru, int value)
{
    m_storage.at(std::string(idVru)).sampleRate = value;
}

[[nodiscard]] int Storage::getSampleRate(std::string_view idVru) const
{
    return m_storage.at(std::string(idVru)).sampleRate;
}

void Storage::setPrearrangedFreqs(std::string_view idVru, FspMap value)
{
    m_storage.at(std::string(idVru)).prearrangedFreqList = std::move(value);
}

void Storage::setHfChannelNum(std::string_view idVru, int value)
{
    m_storage.at(std::string(idVru)).hfChannelNum = value;
}

void Storage::setFreq(std::string_view idVru, long long freq, int hfChannelNum)
{
    if (hfChannelNum != m_storage.at(std::string(idVru)).hfChannelNum) {
        throw std::runtime_error("incorrect hfChannelNum");
    }
    m_storage.at(std::string(idVru)).freq = freq;
}

void Storage::useFreqForPrearranged(std::string_view idVru, short prearrangedFreqNum, int hfChannelNum)
{
    if (hfChannelNum != m_storage.at(std::string(idVru)).hfChannelNum) {
        throw std::runtime_error("incorrect hfChannelNum");
    }
    m_storage.at(std::string(idVru)).prearrangedFreqNum = prearrangedFreqNum;
}

void Storage::setBandWidth(std::string_view idVru, int value)
{
    m_storage.at(std::string(idVru)).bandWidth = value;
}

[[nodiscard]] int Storage::getBandWidth(std::string_view idVru) const
{
    return m_storage.at(std::string(idVru)).bandWidth;
}

}   // namespace vru_srv::vru
