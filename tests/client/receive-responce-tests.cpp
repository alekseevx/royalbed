#include <gtest/gtest.h>

#include "nhope/async/ao-context.h"
#include "nhope/async/async-invoke.h"
#include "nhope/async/thread-executor.h"
#include "nhope/io/io-device.h"
#include "nhope/io/pushback-reader.h"
#include "nhope/io/string-reader.h"

#include "royalbed/client/detail/receive-responce.h"
#include "royalbed/client/headers.h"
#include "royalbed/client/http-error.h"

#include "helpers/bytes.h"
#include "helpers/iodevs.h"

namespace {

using namespace std::literals;
using namespace royalbed::client;
using namespace royalbed::client::detail;

}   // namespace

TEST(ReceiveResponce, OnlyStatusLine)   // NOLINT
{
    constexpr auto rawResponce = "HTTP/1.1 200 Status text\r\n\r\n";

    nhope::ThreadExecutor executor;
    nhope::AOContext aoCtx(executor);

    auto conn = nhope::PushbackReader::create(aoCtx, nhope::StringReader::create(aoCtx, rawResponce));
    receiveResponce(aoCtx, *conn)
      .then([](auto resp) {
          EXPECT_EQ(resp.status, 200);
          EXPECT_EQ(resp.statusMessage, "Status text");
          EXPECT_TRUE(resp.headers.empty());

          return nhope::readAll(std::move(resp.body));
      })
      .then([](auto bodyData) {
          EXPECT_TRUE(bodyData.empty());
      })
      .get();
}

TEST(ReceiveResponce, OnlyHeaders)   // NOLINT
{
    constexpr auto rawResponce = "HTTP/1.1 201\r\n"
                                 "Header1: Value1\r\n"
                                 "Header2: Value2\r\n"
                                 "\r\n";

    nhope::ThreadExecutor executor;
    nhope::AOContext aoCtx(executor);
    auto conn = nhope::PushbackReader::create(aoCtx, nhope::StringReader::create(aoCtx, rawResponce));

    receiveResponce(aoCtx, *conn)
      .then([](auto resp) {
          EXPECT_EQ(resp.status, 201);
          EXPECT_EQ(resp.statusMessage, "");
          EXPECT_EQ(resp.headers, Headers({
                                    {"Header1", "Value1"},
                                    {"Header2", "Value2"},
                                  }));

          return nhope::readAll(std::move(resp.body));
      })
      .then([](auto bodyData) {
          EXPECT_TRUE(bodyData.empty());
      })
      .get();
}

TEST(ReceiveResponce, BadResponce)   // NOLINT
{
    constexpr auto rawResponce = "HTP/1.1 201\r\n";

    nhope::ThreadExecutor executor;
    nhope::AOContext aoCtx(executor);
    auto conn = nhope::PushbackReader::create(aoCtx, nhope::StringReader::create(aoCtx, rawResponce));

    auto future = receiveResponce(aoCtx, *conn);

    // FIXME: Need more suitable error
    EXPECT_THROW(future.get(), HttpError);   // NOLINT
}

TEST(ReceiveResponce, IncompleteResponce)   // NOLINT
{
    constexpr auto rawResponce = "HTP/1.1 201\r";

    nhope::ThreadExecutor executor;
    nhope::AOContext aoCtx(executor);
    auto conn = nhope::PushbackReader::create(aoCtx, nhope::StringReader::create(aoCtx, rawResponce));

    auto future = receiveResponce(aoCtx, *conn);

    // FIXME: Need more suitable error
    EXPECT_THROW(future.get(), HttpError);   // NOLINT
}

TEST(ReceiveResponce, IOError)   // NOLINT
{
    nhope::ThreadExecutor executor;
    nhope::AOContext aoCtx(executor);

    auto conn = nhope::PushbackReader::create(
      aoCtx,                                                                        //
      nhope::concat(aoCtx, nhope::StringReader::create(aoCtx, "HTTP/1.1 200 OK"),   // Begining of the responce header
                    IOErrorReader::create(aoCtx))                                   // IOError
    );

    auto future = receiveResponce(aoCtx, *conn);

    EXPECT_THROW(future.get(), std::system_error);   // NOLINT
}

TEST(ReceiveResponce, BodyReader_IOError)   // NOLINT
{
    nhope::ThreadExecutor executor;
    nhope::AOContext aoCtx(executor);

    auto conn = nhope::PushbackReader::create(
      aoCtx,                                                                      //
      nhope::concat(aoCtx,                                                        //
                    nhope::StringReader::create(aoCtx, "HTTP/1.1 200 OK\r\n"      // Begining of the responce header
                                                       "Content-Length: 10\r\n"   //
                                                       "\r\n"                     //
                                                       "123456"),                 //
                    IOErrorReader::create(aoCtx))                                 // IOError
    );

    auto future = receiveResponce(aoCtx, *conn).then([](auto resp) {
        return nhope::readAll(std::move(resp.body));
    });

    EXPECT_THROW(future.get(), std::system_error);   // NOLINT
}

TEST(ReceiveResponce, Cancel)   // NOLINT
{
    nhope::ThreadExecutor executor;
    nhope::AOContext aoCtx(executor);

    auto conn = nhope::PushbackReader::create(
      aoCtx,                                                               //
      nhope::concat(aoCtx,                                                 //
                    nhope::StringReader::create(aoCtx, "HTTP/1.1 200 "),   // Begining of the responce header
                    SlowReader::create(aoCtx))                             // long reading
    );

    auto future = receiveResponce(aoCtx, *conn);

    EXPECT_FALSE(future.waitFor(100ms));

    aoCtx.close();

    EXPECT_THROW(future.get(), nhope::AsyncOperationWasCancelled);   // NOLINT
}

TEST(ReceiveResponce, BodyReader_Cancel)   // NOLINT
{
    nhope::ThreadExecutor executor;
    nhope::AOContext aoCtx(executor);

    auto conn = nhope::PushbackReader::create(
      aoCtx,                                                                      //
      nhope::concat(aoCtx,                                                        //
                    nhope::StringReader::create(aoCtx, "HTTP/1.1 200 OK\r\n"      // Begining of the responce header
                                                       "Content-Length: 10\r\n"   //
                                                       "\r\n"                     //
                                                       "123456"),                 //
                    SlowReader::create(aoCtx))                                    // long body reading
    );

    auto future = receiveResponce(aoCtx, *conn).then([](auto resp) {
        return nhope::readAll(std::move(resp.body));
    });

    aoCtx.close();

    EXPECT_THROW(future.get(), nhope::AsyncOperationWasCancelled);   // NOLINT
}

TEST(ReceiveResponce, TwoResponce)   // NOLINT
{
    nhope::ThreadExecutor executor;
    nhope::AOContext aoCtx(executor);

    auto conn = nhope::PushbackReader::create(   //
      aoCtx,                                     //
      nhope::StringReader::create(aoCtx, "HTTP/1.1 200 OK\r\n"
                                         "Content-Length: 10\r\n"
                                         "\r\n"
                                         "first-body"
                                         "HTTP/1.1 200 OK\r\n"
                                         "Content-Length: 11\r\n"
                                         "\r\n"
                                         "second-body"));

    receiveResponce(aoCtx, *conn)
      .then([](auto resp) {
          return nhope::readAll(std::move(resp.body));
      })
      .then([&](auto content) {
          EXPECT_EQ(asString(content), "first-body");

          return receiveResponce(aoCtx, *conn).then([](auto resp) {
              return nhope::readAll(std::move(resp.body));
          });
      })
      .then([](auto content) {
          EXPECT_EQ(asString(content), "second-body");
      })
      .get();
}
