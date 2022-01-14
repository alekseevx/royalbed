#include <memory>
#include <string>
#include <utility>

#include "gtest/gtest.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/null_sink.h"

#include "nhope/async/ao-context.h"
#include "nhope/async/async-invoke.h"
#include "nhope/async/future.h"
#include "nhope/async/thread-executor.h"
#include "nhope/io/io-device.h"
#include "nhope/io/pushback-reader.h"
#include "nhope/io/string-reader.h"
#include "nhope/io/tcp.h"

#include "royalbed/client/detail/receive-responce.h"
#include "royalbed/client/request.h"
#include "royalbed/client/detail/send-request.h"

#include "royalbed/server/http-status.h"
#include "royalbed/server/server.h"
#include "royalbed/server/router.h"

#include "helpers/bytes.h"
#include "helpers/logger.h"

namespace {
using namespace royalbed;
using namespace royalbed::server;
constexpr auto port = 7890;

}   // namespace

TEST(Server, Echo)   // NOLINT
{
    constexpr auto iterCount = 200;

    nhope::ThreadExecutor executor;
    nhope::AOContext aoCtx(executor);

    const auto send = [&aoCtx](const std::string& content) {
        using nhope::PushbackReader;
        using nhope::StringReader;
        using nhope::TcpSocket;

        auto sock = TcpSocket::connect(aoCtx, "127.0.0.1", port).get();
        client::detail::sendRequest(aoCtx,
                                    {
                                      .method = "GET",
                                      .uri = {.path = "/echo"},
                                      .headers =
                                        {
                                          {"Content-Length", std::to_string(content.size())},
                                        },
                                      .body = StringReader::create(aoCtx, content),
                                    },
                                    *sock)
          .get();

        auto pushbackReader = PushbackReader::create(aoCtx, *sock);
        auto resp = client::detail::receiveResponce(aoCtx, *pushbackReader).get();

        return asString(nhope::readAll(*resp.body).get());
    };

    auto router = Router();
    router.get("/echo", [](RequestContext& ctx) {
        ctx.responce.status = HttpStatus::Ok;
        ctx.responce.body = std::move(ctx.request.body);
        ctx.responce.headers = ctx.request.headers;
        return nhope::makeReadyFuture();
    });

    auto srv = Server::start(aoCtx, {
                                      .bindAddress = "127.0.0.1",
                                      .port = port,
                                      .router = std::move(router),
                                      .log = nullLogger(),
                                    });

    for (int i = 0; i < iterCount; ++i) {
        const auto data = fmt::format("test_{}", i);
        EXPECT_EQ(send(data), data);
    }
}
