#include <chrono>

#include "gtest/gtest.h"
#include <string>
#include <thread>
#include "spdlog/spdlog.h"

#include "nhope/async/event.h"
#include "nhope/async/thread-executor.h"
#include "nhope/io/null-device.h"

#include "royalbed/server/detail/connection.h"
#include "royalbed/server/router.h"

#include "helpers/logger.h"
#include "helpers/iodevs.h"

namespace {
using namespace std::literals;
using namespace royalbed::server;
using namespace royalbed::server::detail;

constexpr auto etalonConnectionNum = 0xFFEE;
constexpr auto etalonSessionNum = 0xEEFF;

class TestConnectionCtx final : public ConnectionCtx
{
public:
    explicit TestConnectionCtx(Router&& router)
      : m_router(std::move(router))
    {}

    [[nodiscard]] const Router& router() const noexcept override
    {
        return m_router;
    }

    SessionAttr startSession(std::uint32_t connectionNum) override
    {
        EXPECT_EQ(etalonConnectionNum, connectionNum);
        return {
          .num = etalonSessionNum,
          .log = nullLogger(),
        };
    }

    void sessionFinished(std::uint32_t sessionNum) override
    {
        EXPECT_EQ(etalonSessionNum, sessionNum);
        m_finishedSessionSum += sessionNum;
    }

    void connectionClosed(std::uint32_t connectionNum) override
    {
        EXPECT_EQ(etalonConnectionNum, connectionNum);
        m_connectionNumFinished += connectionNum;
        m_event.set();
    }

    bool wait(std::chrono::nanoseconds timeout)
    {
        return m_event.waitFor(timeout);
    }

    [[nodiscard]] std::uint64_t finishedSessionSum() const noexcept
    {
        return m_finishedSessionSum;
    }
    [[nodiscard]] std::uint64_t finishedConnectionSum() const noexcept
    {
        return m_connectionNumFinished;
    }

private:
    Router m_router;
    nhope::Event m_event;
    std::uint64_t m_finishedSessionSum{};
    std::uint64_t m_connectionNumFinished{};
};

}   // namespace

TEST(Connection, FullLiveCycle)   // NOLINT
{
    TestConnectionCtx ctx{Router()};

    auto executor = nhope::ThreadExecutor();
    auto aoCtx = nhope::AOContext(executor);

    aoCtx.exec([&] {
        openConnection(aoCtx, ConnectionParams{
                                .num = etalonConnectionNum,
                                .keepAlive = {},
                                .ctx = ctx,
                                .log = nullLogger(),
                                .sock = NullSock::create(aoCtx),
                              });
    });
    std::this_thread::sleep_for(std::chrono::seconds(4));
    EXPECT_TRUE(ctx.wait(1s));
}

TEST(Connection, Cancel)   // NOLINT
{
    TestConnectionCtx ctx{Router()};

    auto executor = nhope::ThreadExecutor();
    auto aoCtx = nhope::AOContext(executor);
    openConnection(aoCtx, ConnectionParams{
                            .num = etalonConnectionNum,
                            .keepAlive = {},
                            .ctx = ctx,
                            .log = nullLogger(),
                            .sock = SlowSock::create(aoCtx),
                          });
    EXPECT_FALSE(ctx.wait(20ms));

    aoCtx.close();

    EXPECT_TRUE(ctx.wait(1s));
}

TEST(Connection, KeepAlive)   // NOLINT
{
    TestConnectionCtx ctx{Router()};

    auto executor = nhope::ThreadExecutor();
    auto aoCtx = nhope::AOContext(executor);

    std::string reqStr = "GET /path HTTP/1.1\r\nHost: 127.0.0.1:2\r\n\r\n";
    reqStr += reqStr;

    openConnection(aoCtx, ConnectionParams{
                            .num = etalonConnectionNum,
                            .keepAlive =
                              {
                                .timeout = 1s,
                                .requestsCount = 2,
                              },
                            .ctx = ctx,
                            .log = nullLogger(),
                            .sock = EchoSock::create(aoCtx, reqStr),
                          });

    EXPECT_TRUE(ctx.wait(2s));
    EXPECT_EQ(ctx.finishedSessionSum(), etalonSessionNum * 2);
    EXPECT_EQ(ctx.finishedConnectionSum(), etalonConnectionNum);
}

TEST(Connection, KeepAliveTimeout)   // NOLINT
{
    TestConnectionCtx ctx{Router()};

    auto executor = nhope::ThreadExecutor();
    auto aoCtx = nhope::AOContext(executor);

    std::string reqStr = "GET /path HTTP/1.1\r\nHost: 127.0.0.1:2\r\n\r\n";
    reqStr += reqStr;

    openConnection(aoCtx, ConnectionParams{
                            .num = etalonConnectionNum,
                            .keepAlive =
                              {
                                .timeout = 1s,
                                .requestsCount = etalonSessionNum,
                              },
                            .ctx = ctx,
                            .log = nullLogger(),
                            .sock = EchoSock::create(aoCtx, reqStr),
                          });

    EXPECT_TRUE(ctx.wait(2s));
}
