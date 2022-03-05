#include <cstdlib>

#include "asio/io_context.hpp"

#include "cmrc/cmrc.hpp"

#include "nhope/async/ao-context.h"
#include "nhope/async/io-context-executor.h"

#include "royalbed/server/http-status.h"
#include "royalbed/server/redoc.h"
#include "royalbed/server/server.h"
#include "royalbed/server/router.h"
#include "royalbed/server/swagger.h"

#include "spdlog/sinks/null_sink.h"
#include "spdlog/spdlog.h"

#include "endpoints/all.h"

namespace {

using namespace royalbed::server;
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

        auto openApiFS = cmrc::vru_srv::get_filesystem();
        swagger(router, openApiFS, "api/example.yml");
        redoc(router, openApiFS, "api/example.yml");

        auto server = Server::start(aoCtx, {
                                             .bindAddress = "0.0.0.0",
                                             .port = portSrv,
                                             .router = std::move(router),
                                             .log = spdlog::default_logger()->clone("httpsrv"),
                                           });

        auto workGuard = asio::make_work_guard(ioCtx);
        ioCtx.run();
        return EXIT_SUCCESS;
    } catch (const std::exception& e) {
        spdlog::error("{0}", e.what());
        return EXIT_FAILURE;
    } catch (...) {
        return EXIT_FAILURE;
    }
}
