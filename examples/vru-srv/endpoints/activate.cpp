#include "../vru/storage.h"

#include "activate.h"
#include "params.h"

namespace vru_srv::endpoints {

void publicActivateEndpoint(royalbed::server::Router& router)
{
    router.post(":vruId/activate", [](const VruId& vruId) {
        auto& storage = vru::Storage::instance();
        storage.activate(*vruId);
    });
}

}   // namespace vru_srv::endpoints
