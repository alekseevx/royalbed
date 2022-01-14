#include <cstdlib>

#include "asio/io_context.hpp"

#include "cmrc/cmrc.hpp"

#include "nhope/async/ao-context.h"
#include "nhope/async/io-context-executor.h"

#include "royalbed/server/http-status.h"
#include "royalbed/server/server.h"
#include "royalbed/server/router.h"
#include "royalbed/server/swagger.h"

#include "spdlog/spdlog.h"

#include "endpoints/all.h"

namespace {

using namespace royalbed::server;
constexpr auto ipSrv = "127.0.0.1";
constexpr auto portSrv = 8080;

}   // namespace

CMRC_DECLARE(vru_srv);

int main()
{
    try {
        auto ioCtx = asio::io_context();
        auto executor = nhope::IOContextSequenceExecutor(ioCtx);
        auto aoCtx = nhope::AOContext(executor);

        auto router = Router();
        vru_srv::endpoints::publicEndpoints(router);

        auto swaggerFs = cmrc::vru_srv::get_filesystem();
        swagger(router, swaggerFs, "api/example.yml");

        auto params = ServerParams{
          .bindAddress = ipSrv,
          .port = portSrv,
          .router = std::move(router),
          .log = spdlog::default_logger(),
        };

        start(aoCtx, std::move(params));

        auto workGuard = asio::make_work_guard(ioCtx);
        ioCtx.run();
    } catch (const std::exception& e) {
        spdlog::error("{0}", e.what());
        return EXIT_FAILURE;
    } catch (...) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
