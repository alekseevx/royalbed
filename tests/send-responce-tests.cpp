#include <gtest/gtest.h>

#include "nhope/async/ao-context.h"
#include "nhope/async/async-invoke.h"
#include "nhope/async/thread-executor.h"
#include "nhope/io/string-reader.h"
#include "nhope/io/string-writter.h"

#include "royalbed/detail/responce.h"
#include "royalbed/http-status.h"
#include "royalbed/send-responce.h"

#include "helpers/iodevs.h"

namespace {

using namespace std::literals;
using namespace royalbed;
using namespace royalbed::detail;

}   // namespace

TEST(SendResponce, SendResponceWithoutBody)   // NOLINT
{
    constexpr auto etalone = "HTTP/1.1 200 OK\r\nHeader1: Value1\r\n\r\n"sv;

    nhope::ThreadExecutor executor;
    nhope::AOContext aoCtx(executor);

    auto resp = Responce::create();
    resp->status = HttpStatus::Ok;
    resp->headers = {
      {"Header1", "Value1"},
    };

    auto dev = nhope::StringWritter::create(aoCtx);

    const auto n = sendResponce(aoCtx, std::move(resp), *dev).get();

    EXPECT_EQ(n, etalone.size());
    EXPECT_EQ(dev->takeContent(), etalone);
}

TEST(SendResponce, SendResponceWithCustomStatusMessage)   // NOLINT
{
    constexpr auto etalone = "HTTP/1.1 200 COOL\r\nHeader1: Value1\r\n\r\n"sv;

    nhope::ThreadExecutor executor;
    nhope::AOContext aoCtx(executor);

    auto resp = Responce::create();
    resp->status = HttpStatus::Ok;
    resp->statusMessage = "COOL";
    resp->headers = {
      {"Header1", "Value1"},
    };

    auto dev = nhope::StringWritter::create(aoCtx);

    const auto n = sendResponce(aoCtx, std::move(resp), *dev).get();

    EXPECT_EQ(n, etalone.size());
    EXPECT_EQ(dev->takeContent(), etalone);
}

TEST(SendResponce, SendResponceWithBody)   // NOLINT
{
    constexpr auto etalone = "HTTP/1.1 200 OK\r\nContent-Length: 10\r\n\r\n1234567890"sv;

    nhope::ThreadExecutor executor;
    nhope::AOContext aoCtx(executor);

    auto resp = Responce::create();
    resp->headers = {
      {"Content-Length", "10"},
    };
    resp->body = nhope::StringReader::create(aoCtx, "1234567890");

    auto dev = nhope::StringWritter::create(aoCtx);

    const auto n = sendResponce(aoCtx, std::move(resp), *dev).get();

    EXPECT_EQ(n, etalone.size());
    EXPECT_EQ(dev->takeContent(), etalone);
}

TEST(SendResponce, IOError)   // NOLINT
{
    nhope::ThreadExecutor executor;
    nhope::AOContext aoCtx(executor);

    auto resp = Responce::create();
    resp->headers = {
      {"Header1", "Value1"},
    };

    auto dev = IOErrorWritter::create(aoCtx);

    auto future = sendResponce(aoCtx, std::move(resp), *dev);

    EXPECT_THROW(future.get(), std::system_error);   // NOLINT
}

TEST(SendResponce, Cancel)   // NOLINT
{
    nhope::ThreadExecutor executor;
    nhope::AOContext aoCtx(executor);

    auto resp = Responce::create();
    resp->headers = {
      {"Header1", "Value1"},
    };

    auto dev = SlowWritter::create(aoCtx);

    auto future = sendResponce(aoCtx, std::move(resp), *dev);

    aoCtx.close();

    EXPECT_THROW(future.get(), nhope::AsyncOperationWasCancelled);   // NOLINT
}
