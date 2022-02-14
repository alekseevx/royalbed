#include <chrono>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>

#include "gtest/gtest.h"

#include "nhope/async/event.h"
#include "spdlog/spdlog.h"

#include "nhope/async/ao-context-close-handler.h"
#include "nhope/async/ao-context.h"
#include "nhope/async/async-invoke.h"
#include "nhope/async/future.h"
#include "nhope/async/thread-executor.h"
#include "nhope/io/io-device.h"
#include "nhope/io/pushback-reader.h"
#include "nhope/io/string-reader.h"
#include "nhope/io/string-writter.h"

#include "royalbed/server/detail/session.h"
#include "royalbed/server/http-error.h"
#include "royalbed/server/http-status.h"
#include "royalbed/server/request-context.h"
#include "royalbed/server/router.h"

#include "helpers/iodevs.h"
#include "helpers/logger.h"

namespace {
using namespace std::literals;
using namespace royalbed::server;
using namespace royalbed::server::detail;

class TestSessionCtx final : public SessionCtx
{
public:
    explicit TestSessionCtx(Router&& router)
      : m_router(std::move(router))
    {}

    [[nodiscard]] const Router& router() const noexcept override
    {
        return m_router;
    }

    void sessionReceivedRequest(std::uint32_t /*sessionNum*/) noexcept override
    {}

    void sessionFinished(std::uint32_t /*sessionNum*/, bool /*success*/) noexcept override
    {
        m_event.set();
    }

    bool wait(std::chrono::nanoseconds timeout)
    {
        return m_event.waitFor(timeout);
    }

private:
    Router m_router;
    nhope::Event m_event;
};

nhope::PushbackReaderPtr inputStream(nhope::AOContext& aoCtx, std::string request)
{
    using namespace nhope;
    return PushbackReader::create(aoCtx, StringReader::create(aoCtx, std::move(request)));
}

}   // namespace

TEST(Session, OnlyHandler)   // NOLINT
{
    auto router = Router();
    router.get("/path", [](RequestContext& ctx) {
        EXPECT_EQ(ctx.request.method, "GET");
        EXPECT_EQ(ctx.request.uri.toString(), "/path?k=v#fragment");
        ctx.responce = {
          .status = HttpStatus::Ok,
        };
        return nhope::makeReadyFuture();
    });

    auto executor = nhope::ThreadExecutor();
    auto aoCtx = nhope::AOContext(executor);
    TestSessionCtx testSessionCtx(std::move(router));

    auto in = inputStream(aoCtx, "GET /path?k=v#fragment HTTP/1.1\r\n\r\n");
    auto out = nhope::StringWritter::create(aoCtx);

    startSession(aoCtx, SessionParams{
                          .ctx = testSessionCtx,
                          .in = *in,
                          .out = *out,
                          .log = nullLogger(),
                        });

    EXPECT_TRUE(testSessionCtx.wait(1s));

    const auto responce = out->takeContent();
    EXPECT_EQ(responce, "HTTP/1.1 200 OK\r\nConnection: close\r\n\r\n");
}

TEST(Session, OnlyMiddlewares)   // NOLINT
{
    auto router = Router();
    std::atomic<int> middlewareCounter = 0;
    router.addMiddleware([&](RequestContext& /*ctx*/) {
        ++middlewareCounter;
        return nhope::makeReadyFuture<bool>(true);
    });

    router.addMiddleware([&](RequestContext& /*ctx*/) {
        ++middlewareCounter;
        return nhope::makeReadyFuture<bool>(false);
    });

    std::atomic<int> handlerCounter = 0;
    router.get("/path", [&](RequestContext& /*ctx*/) {
        ++handlerCounter;
        return nhope::makeReadyFuture();
    });

    auto executor = nhope::ThreadExecutor();
    auto aoCtx = nhope::AOContext(executor);
    TestSessionCtx testSessionCtx(std::move(router));

    auto in = inputStream(aoCtx, "GET /path?k=v#fragment HTTP/1.1\r\n\r\n");
    auto out = nhope::StringWritter::create(aoCtx);

    startSession(aoCtx, SessionParams{
                          .ctx = testSessionCtx,
                          .in = *in,
                          .out = *out,
                          .log = nullLogger(),
                        });

    EXPECT_TRUE(testSessionCtx.wait(1s));

    EXPECT_EQ(middlewareCounter, 2);
    EXPECT_EQ(handlerCounter, 0);
}

TEST(Session, MiddlewaresAndHandler)   // NOLINT
{
    auto router = Router();
    std::atomic<int> middlewareCounter = 0;
    router.addMiddleware([&](RequestContext& /*ctx*/) {
        ++middlewareCounter;
        return nhope::makeReadyFuture<bool>(true);
    });

    router.addMiddleware([&](RequestContext& /*ctx*/) {
        ++middlewareCounter;
        return nhope::makeReadyFuture<bool>(true);
    });

    std::atomic<int> handlerCounter = 0;
    router.get("/path", [&](RequestContext& /*ctx*/) {
        ++handlerCounter;
        return nhope::makeReadyFuture();
    });

    auto executor = nhope::ThreadExecutor();
    auto aoCtx = nhope::AOContext(executor);
    TestSessionCtx testSessionCtx(std::move(router));

    auto in = inputStream(aoCtx, "GET /path?k=v#fragment HTTP/1.1\r\n\r\n");
    auto out = nhope::StringWritter::create(aoCtx);

    startSession(aoCtx, SessionParams{
                          .ctx = testSessionCtx,
                          .in = *in,
                          .out = *out,
                          .log = nullLogger(),
                        });

    EXPECT_TRUE(testSessionCtx.wait(1s));

    EXPECT_EQ(middlewareCounter, 2);
    EXPECT_EQ(handlerCounter, 1);
}

TEST(Session, ExceptionInHandler)   // NOLINT
{
    auto router = Router();
    router.get("/path", [] {
        throw HttpError(HttpStatus::BadRequest, "XXX");
    });

    auto executor = nhope::ThreadExecutor();
    auto aoCtx = nhope::AOContext(executor);
    TestSessionCtx testSessionCtx(std::move(router));

    auto in = inputStream(aoCtx, "GET /path?k=v#fragment HTTP/1.1\r\n\r\n");
    auto out = nhope::StringWritter::create(aoCtx);

    startSession(aoCtx, SessionParams{
                          .ctx = testSessionCtx,
                          .in = *in,
                          .out = *out,
                          .log = nullLogger(),
                        });

    EXPECT_TRUE(testSessionCtx.wait(1s));

    const auto responce = out->takeContent();
    EXPECT_EQ(responce, "HTTP/1.1 400 XXX\r\nConnection: close\r\n\r\n");
}

TEST(Session, ExceptionInMiddleware)   // NOLINT
{
    auto router = Router();
    router.addMiddleware([](RequestContext&) -> nhope::Future<bool> {
        throw std::runtime_error("XXX");
    });
    router.get("/path", [](RequestContext& ctx) {
        ctx.responce = {
          .status = HttpStatus::Ok,
        };
        return nhope::makeReadyFuture();
    });

    auto executor = nhope::ThreadExecutor();
    auto aoCtx = nhope::AOContext(executor);
    TestSessionCtx testSessionCtx(std::move(router));

    auto in = inputStream(aoCtx, "GET /path?k=v#fragment HTTP/1.1\r\n\r\n");
    auto out = nhope::StringWritter::create(aoCtx);

    startSession(aoCtx, SessionParams{
                          .ctx = testSessionCtx,
                          .in = *in,
                          .out = *out,
                          .log = nullLogger(),
                        });

    EXPECT_TRUE(testSessionCtx.wait(1s));

    const auto responce = out->takeContent();
    EXPECT_TRUE(responce.find("HTTP/1.1 500 Internal Server Error") != std::string::npos);
}

TEST(Session, Close)   // NOLINT
{
    auto router = Router();

    auto executor = nhope::ThreadExecutor();
    auto aoCtx = nhope::AOContext(executor);
    TestSessionCtx testSessionCtx(std::move(router));

    auto in = nhope::PushbackReader::create(aoCtx, SlowSock::create(aoCtx));
    auto out = nhope::StringWritter::create(aoCtx);

    startSession(aoCtx, SessionParams{
                          .ctx = testSessionCtx,
                          .in = *in,
                          .out = *out,
                          .log = nullLogger(),
                        });

    EXPECT_FALSE(testSessionCtx.wait(20ms));

    aoCtx.close();

    EXPECT_TRUE(testSessionCtx.wait(1s));
}