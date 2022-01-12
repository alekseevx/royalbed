#include <chrono>

#include "gtest/gtest.h"
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
    }

    void connectionClosed(std::uint32_t connectionNum) override
    {
        EXPECT_EQ(etalonConnectionNum, connectionNum);
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

}   // namespace

TEST(Connection, FullLiveCycle)   // NOLINT
{
    TestConnectionCtx ctx{Router()};

    auto executor = nhope::ThreadExecutor();
    auto aoCtx = nhope::AOContext(executor);

    aoCtx.exec([&] {
        openConnection(aoCtx, ConnectionParams{
                                .num = etalonConnectionNum,
                                .ctx = ctx,
                                .log = nullLogger(),
                                .sock = NullSock::create(aoCtx),
                              });
    });

    EXPECT_TRUE(ctx.wait(1s));
}

TEST(Connection, Cancel)   // NOLINT
{
    TestConnectionCtx ctx{Router()};

    auto executor = nhope::ThreadExecutor();
    auto aoCtx = nhope::AOContext(executor);
    openConnection(aoCtx, ConnectionParams{
                            .num = etalonConnectionNum,
                            .ctx = ctx,
                            .log = nullLogger(),
                            .sock = SlowSock::create(aoCtx),
                          });
    EXPECT_FALSE(ctx.wait(20ms));

    aoCtx.close();

    EXPECT_TRUE(ctx.wait(1s));
}
