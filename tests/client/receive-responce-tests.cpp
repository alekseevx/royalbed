#include <gtest/gtest.h>

#include "nhope/async/ao-context.h"
#include "nhope/async/async-invoke.h"
#include "nhope/async/thread-executor.h"
#include "nhope/io/io-device.h"
#include "nhope/io/pushback-reader.h"
#include "nhope/io/string-reader.h"

#include "royalbed/client/detail/receive-response.h"
#include "royalbed/client/headers.h"
#include "royalbed/client/http-error.h"

#include "helpers/bytes.h"
#include "helpers/iodevs.h"

namespace {

using namespace std::literals;
using namespace royalbed::client;
using namespace royalbed::client::detail;

}   // namespace

TEST(ReceiveResponse, OnlyStatusLine)   // NOLINT
{
    constexpr auto rawResponse = "HTTP/1.1 200 Status text\r\n\r\n";

    nhope::ThreadExecutor executor;
    nhope::AOContext aoCtx(executor);

    auto conn = nhope::PushbackReader::create(aoCtx, nhope::StringReader::create(aoCtx, rawResponse));
    receiveResponse(aoCtx, *conn)
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

TEST(ReceiveResponse, OnlyHeaders)   // NOLINT
{
    constexpr auto rawResponse = "HTTP/1.1 201\r\n"
                                 "Header1: Value1\r\n"
                                 "Header2: Value2\r\n"
                                 "\r\n";

    nhope::ThreadExecutor executor;
    nhope::AOContext aoCtx(executor);
    auto conn = nhope::PushbackReader::create(aoCtx, nhope::StringReader::create(aoCtx, rawResponse));

    receiveResponse(aoCtx, *conn)
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

TEST(ReceiveResponse, Badresponse)   // NOLINT
{
    constexpr auto rawResponse = "HTP/1.1 201\r\n";

    nhope::ThreadExecutor executor;
    nhope::AOContext aoCtx(executor);
    auto conn = nhope::PushbackReader::create(aoCtx, nhope::StringReader::create(aoCtx, rawResponse));

    auto future = receiveResponse(aoCtx, *conn);

    // FIXME: Need more suitable error
    EXPECT_THROW(future.get(), HttpError);   // NOLINT
}

TEST(ReceiveResponse, Incompleteresponse)   // NOLINT
{
    constexpr auto rawResponse = "HTP/1.1 201\r";

    nhope::ThreadExecutor executor;
    nhope::AOContext aoCtx(executor);
    auto conn = nhope::PushbackReader::create(aoCtx, nhope::StringReader::create(aoCtx, rawResponse));

    auto future = receiveResponse(aoCtx, *conn);

    // FIXME: Need more suitable error
    EXPECT_THROW(future.get(), HttpError);   // NOLINT
}

TEST(ReceiveResponse, IOError)   // NOLINT
{
    nhope::ThreadExecutor executor;
    nhope::AOContext aoCtx(executor);

    auto conn = nhope::PushbackReader::create(
      aoCtx,                                                                        //
      nhope::concat(aoCtx, nhope::StringReader::create(aoCtx, "HTTP/1.1 200 OK"),   // Begining of the response header
                    BrokenSock::create(aoCtx))                                      // IOError
    );

    auto future = receiveResponse(aoCtx, *conn);

    EXPECT_THROW(future.get(), std::system_error);   // NOLINT
}

TEST(ReceiveResponse, BodyReader_IOError)   // NOLINT
{
    nhope::ThreadExecutor executor;
    nhope::AOContext aoCtx(executor);

    auto conn = nhope::PushbackReader::create(
      aoCtx,                                                                      //
      nhope::concat(aoCtx,                                                        //
                    nhope::StringReader::create(aoCtx, "HTTP/1.1 200 OK\r\n"      // Begining of the response header
                                                       "Content-Length: 10\r\n"   //
                                                       "\r\n"                     //
                                                       "123456"),                 //
                    BrokenSock::create(aoCtx))                                    // IOError
    );

    auto future = receiveResponse(aoCtx, *conn).then([](auto resp) {
        return nhope::readAll(std::move(resp.body));
    });

    EXPECT_THROW(future.get(), std::system_error);   // NOLINT
}

TEST(Receiveresponse, Cancel)   // NOLINT
{
    nhope::ThreadExecutor executor;
    nhope::AOContext aoCtx(executor);

    auto conn = nhope::PushbackReader::create(
      aoCtx,                                                               //
      nhope::concat(aoCtx,                                                 //
                    nhope::StringReader::create(aoCtx, "HTTP/1.1 200 "),   // Begining of the response header
                    SlowSock::create(aoCtx))                               // long reading
    );

    auto future = receiveResponse(aoCtx, *conn);

    EXPECT_FALSE(future.waitFor(100ms));

    aoCtx.close();

    EXPECT_THROW(future.get(), nhope::AsyncOperationWasCancelled);   // NOLINT
}

TEST(ReceiveResponse, BodyReader_Cancel)   // NOLINT
{
    nhope::ThreadExecutor executor;
    nhope::AOContext aoCtx(executor);

    auto conn = nhope::PushbackReader::create(
      aoCtx,                                                                      //
      nhope::concat(aoCtx,                                                        //
                    nhope::StringReader::create(aoCtx, "HTTP/1.1 200 OK\r\n"      // Begining of the response header
                                                       "Content-Length: 10\r\n"   //
                                                       "\r\n"                     //
                                                       "123456"),                 //
                    SlowSock::create(aoCtx))                                      // long body reading
    );

    auto future = receiveResponse(aoCtx, *conn).then([](auto resp) {
        return nhope::readAll(std::move(resp.body));
    });

    aoCtx.close();

    EXPECT_THROW(future.get(), nhope::AsyncOperationWasCancelled);   // NOLINT
}

TEST(ReceiveResponse, TwoResponse)   // NOLINT
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

    receiveResponse(aoCtx, *conn)
      .then([](auto resp) {
          return nhope::readAll(std::move(resp.body));
      })
      .then([&](auto content) {
          EXPECT_EQ(asString(content), "first-body");

          return receiveResponse(aoCtx, *conn).then([](auto resp) {
              return nhope::readAll(std::move(resp.body));
          });
      })
      .then([](auto content) {
          EXPECT_EQ(asString(content), "second-body");
      })
      .get();
}
