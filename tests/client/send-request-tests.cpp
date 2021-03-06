#include <memory>
#include <system_error>

#include <gtest/gtest.h>

#include "nhope/async/ao-context-error.h"
#include "nhope/async/ao-context.h"
#include "nhope/async/async-invoke.h"
#include "nhope/async/thread-executor.h"
#include "nhope/io/string-reader.h"
#include "nhope/io/string-writter.h"

#include "royalbed/client/detail/send-request.h"
#include "royalbed/client/request.h"

#include "helpers/iodevs.h"

namespace {

using namespace std::literals;
using namespace royalbed::client;
using namespace royalbed::client::detail;

}   // namespace

TEST(SendRequest, SendReqWithoutBody)   // NOLINT
{
    constexpr auto etalone = "GET /file HTTP/1.1\r\nHeader1: Value1\r\n\r\n"sv;

    nhope::ThreadExecutor executor;
    nhope::AOContext aoCtx(executor);

    auto req = Request{
      .method = "GET",
      .uri = {.path = "/file"},
      .headers =
        {
          {"Header1", "Value1"},
        },
    };

    auto dev = nhope::StringWritter::create(aoCtx);

    const auto n = sendRequest(aoCtx, std::move(req), *dev).get();

    EXPECT_EQ(n, etalone.size());
    EXPECT_EQ(dev->takeContent(), etalone);
}

TEST(SendRequest, SendReqWithBody)   // NOLINT
{
    constexpr auto etalone = "PUT /file%20name HTTP/1.1\r\nContent-Length: 10\r\n\r\n1234567890"sv;

    nhope::ThreadExecutor executor;
    nhope::AOContext aoCtx(executor);

    auto req = Request{
      .method = "PUT",
      .uri = {.path = "/file name"},
      .headers =
        {
          {"Content-Length", "10"},
        },
      .body = nhope::StringReader::create(aoCtx, "1234567890"),
    };

    auto dev = nhope::StringWritter::create(aoCtx);

    const auto n = sendRequest(aoCtx, std::move(req), *dev).get();

    EXPECT_EQ(n, etalone.size());
    EXPECT_EQ(dev->takeContent(), etalone);
}

TEST(SendRequest, IOError)   // NOLINT
{
    nhope::ThreadExecutor executor;
    nhope::AOContext aoCtx(executor);

    auto req = Request{
      .method = "GET",
      .uri = {.path = "/file"},
      .headers =
        {
          {"Header1", "Value1"},
        },
    };

    auto dev = BrokenSock::create(aoCtx);

    auto future = sendRequest(aoCtx, std::move(req), *dev);

    EXPECT_THROW(future.get(), std::system_error);   // NOLINT
}

TEST(SendRequest, Cancel)   // NOLINT
{
    nhope::ThreadExecutor executor;
    nhope::AOContext aoCtx(executor);

    auto req = Request{
      .method = "GET",
      .uri = {.path = "/file"},
      .headers =
        {
          {"Header1", "Value1"},
        },
    };

    auto dev = SlowSock::create(aoCtx);

    auto future = sendRequest(aoCtx, std::move(req), *dev);

    aoCtx.close();

    EXPECT_THROW(future.get(), nhope::AsyncOperationWasCancelled);   // NOLINT
}
