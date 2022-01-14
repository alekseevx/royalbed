#pragma once

#include <string>

namespace vru_srv::vru {

enum class Status
{
    Active,
    NotActive,
    Broken
};

std::string toString(Status status);

}   // namespace vru_srv::vru
