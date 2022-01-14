#include "status.h"

namespace vru_srv::vru {

std::string toString(Status status)
{
    if (status == Status::Active) {
        return "Active";
    }
    if (status == Status::NotActive) {
        return "NotActive";
    }

    return "Broken";
}

}   // namespace vru_srv::vru
