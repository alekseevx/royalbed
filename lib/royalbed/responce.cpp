#include "royalbed/detail/responce.h"
#include <memory>

namespace royalbed::detail {

ResponcePtr Responce::create()
{
    return std::make_unique<Responce>();
}

}   // namespace royalbed::detail
