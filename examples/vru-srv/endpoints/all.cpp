#include "../vru/storage.h"

#include "activate.h"
#include "bandwidth.h"
#include "deactivate.h"
#include "freq.h"
#include "params.h"
#include "prearranged-freqs.h"
#include "royalbed/server/router.h"
#include "sample-rate.h"
#include "status.h"

#include "all.h"

namespace vru_srv::endpoints {

namespace {
void publicAllVruEndpoint(royalbed::server::Router& router)
{
    router.get("/", [] {
        const auto& storage = vru::Storage::instance();
        return storage.getAllVru();
    });
}

void publicIdVruEndpoint(royalbed::server::Router& router)
{
    router.get(":vruId", [](const VruId& vruId) {
        const auto& storage = vru::Storage::instance();
        return storage.getAllVru().at(*vruId);
    });
}
}   // namespace

void publicEndpoints(royalbed::server::Router& router)
{
    royalbed::server::Router vruRouter;
    publicAllVruEndpoint(vruRouter);
    publicIdVruEndpoint(vruRouter);
    publicActivateEndpoint(vruRouter);
    publicBandwidthEndpoint(vruRouter);
    publicDeactivateEndpoint(vruRouter);
    publicFreqEndpoint(vruRouter);
    publicPrearrangedFreqsEndpoints(vruRouter);
    publicSampleRateEndpoint(vruRouter);
    publicStatusEndpoint(vruRouter);

    router.use("/api/vru", std::move(vruRouter));
}

}   // namespace vru_srv::endpoints
