#include <gtest/gtest.h>

#include "nhope/async/ao-context.h"
#include "nhope/async/async-invoke.h"
#include "nhope/async/thread-executor.h"
#include "nhope/io/string-reader.h"
#include "nhope/io/string-writter.h"

#include "royalbed/server/detail/send-response.h"
#include "royalbed/server/http-status.h"
#include "royalbed/server/response.h"

#include "helpers/iodevs.h"

namespace {

using namespace std::literals;
using namespace royalbed::server;
using namespace royalbed::server::detail;

}   // namespace

TEST(SendResponse, SendResponseWithoutBody)   // NOLINT
{
    constexpr auto etalone = "HTTP/1.1 200 OK\r\nHeader1: Value1\r\n\r\n"sv;

    nhope::ThreadExecutor executor;
    nhope::AOContext aoCtx(executor);

    auto resp = Response{
      .status = HttpStatus::Ok,
      .headers =
        {
          {"Header1", "Value1"},
        },
    };

    auto dev = nhope::StringWritter::create(aoCtx);

    const auto n = sendResponse(aoCtx, std::move(resp), *dev).get();

    EXPECT_EQ(n, etalone.size());
    EXPECT_EQ(dev->takeContent(), etalone);
}

TEST(SendResponse, SendResponseWithCustomStatusMessage)   // NOLINT
{
    constexpr auto etalone = "HTTP/1.1 200 COOL\r\nHeader1: Value1\r\n\r\n"sv;

    nhope::ThreadExecutor executor;
    nhope::AOContext aoCtx(executor);

    auto resp = Response{
      .status = HttpStatus::Ok,
      .statusMessage = "COOL",
      .headers =
        {
          {"Header1", "Value1"},
        },
    };

    auto dev = nhope::StringWritter::create(aoCtx);

    const auto n = sendResponse(aoCtx, std::move(resp), *dev).get();

    EXPECT_EQ(n, etalone.size());
    EXPECT_EQ(dev->takeContent(), etalone);
}

TEST(SendResponse, SendResponseWithBody)   // NOLINT
{
    constexpr auto etalone = "HTTP/1.1 200 OK\r\nContent-Length: 10\r\n\r\n1234567890"sv;

    nhope::ThreadExecutor executor;
    nhope::AOContext aoCtx(executor);

    auto resp = Response{
      .headers =
        {
          {"Content-Length", "10"},
        },
      .body = nhope::StringReader::create(aoCtx, "1234567890"),
    };

    auto dev = nhope::StringWritter::create(aoCtx);

    const auto n = sendResponse(aoCtx, std::move(resp), *dev).get();

    EXPECT_EQ(n, etalone.size());
    EXPECT_EQ(dev->takeContent(), etalone);
}

TEST(SendResponse, IOError)   // NOLINT
{
    nhope::ThreadExecutor executor;
    nhope::AOContext aoCtx(executor);

    auto resp = Response{
      .headers =
        {
          {"Header1", "Value1"},
        },
    };

    auto dev = BrokenSock::create(aoCtx);

    auto future = sendResponse(aoCtx, std::move(resp), *dev);

    EXPECT_THROW(future.get(), std::system_error);   // NOLINT
}

TEST(SendResponse, Cancel)   // NOLINT
{
    nhope::ThreadExecutor executor;
    nhope::AOContext aoCtx(executor);

    auto resp = Response{
      .statusMessage = "OK",
      .headers =
        {
          {"Header1", "Value1"},
        },
    };

    auto dev = SlowSock::create(aoCtx);

    auto future = sendResponse(aoCtx, std::move(resp), *dev);

    aoCtx.close();

    EXPECT_THROW(future.get(), nhope::AsyncOperationWasCancelled);   // NOLINT
}
