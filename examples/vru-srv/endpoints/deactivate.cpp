#include "../vru/storage.h"

#include "params.h"
#include "deactivate.h"

namespace vru_srv {

void publicDeactivateEndpoint(royalbed::server::Router& router)
{
    router.post(":vruId/deactivate", [](const VruId& vruId) {
        auto& storage = vru::Storage::instance();
        storage.deactivate(*vruId);
    });
}

}   // namespace vru_srv
