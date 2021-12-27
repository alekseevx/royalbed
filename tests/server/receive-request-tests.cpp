#include <gtest/gtest.h>

#include "nhope/async/ao-context-error.h"
#include "nhope/async/ao-context.h"
#include "nhope/async/async-invoke.h"
#include "nhope/async/future.h"
#include "nhope/async/thread-executor.h"
#include "nhope/async/thread-pool-executor.h"
#include "nhope/io/io-device.h"
#include "nhope/io/pushback-reader.h"
#include "nhope/io/string-reader.h"

#include "royalbed/server/http-error.h"
#include "royalbed/server/detail/receive-request.h"

#include "helpers/bytes.h"
#include "helpers/iodevs.h"

namespace {

using namespace std::literals;
using namespace royalbed::server;
using namespace royalbed::server::detail;

}   // namespace

TEST(ReceiveRequest, OnlyMethodLine)   // NOLINT
{
    constexpr auto rawRequest = "GET /path?k=v#fragment HTTP/1.1\r\n\r\n";

    nhope::ThreadExecutor executor;
    nhope::AOContext aoCtx(executor);

    auto conn = nhope::PushbackReader::create(aoCtx, nhope::StringReader::create(aoCtx, rawRequest));
    receiveRequest(aoCtx, *conn)
      .then(aoCtx,
            [](auto req) {
                EXPECT_EQ(req.method, "GET");
                EXPECT_EQ(req.uri.toString(), "/path?k=v#fragment");
                EXPECT_TRUE(req.headers.empty());

                return nhope::readAll(std::move(req.body));
            })
      .then([](auto bodyData) {
          EXPECT_TRUE(bodyData.empty());
      })
      .get();
}

TEST(ReceiveRequest, OnlyHeaders)   // NOLINT
{
    constexpr auto rawRequest = "GET /path HTTP/1.1\r\n"
                                "Header1: Value1\r\n"
                                "Header2: Value2\r\n"
                                "\r\n";

    nhope::ThreadExecutor executor;
    nhope::AOContext aoCtx(executor);

    auto conn = nhope::PushbackReader::create(aoCtx, nhope::StringReader::create(aoCtx, rawRequest));
    receiveRequest(aoCtx, *conn)
      .then([](auto req) {
          EXPECT_EQ(req.method, "GET");
          EXPECT_EQ(req.uri.toString(), "/path");
          EXPECT_EQ(req.headers, Headers({
                                   {"Header1", "Value1"},
                                   {"Header2", "Value2"},
                                 }));

          return nhope::readAll(std::move(req.body));
      })
      .then([](auto bodyData) {
          EXPECT_TRUE(bodyData.empty());
      })
      .get();
}

TEST(ReceiveRequest, BodyReader)   // NOLINT
{
    constexpr auto rawRequest = "GET /path HTTP/1.1\r\n"
                                "Content-Length: 10\r\n"
                                "\r\n"
                                "1234567890";

    nhope::ThreadExecutor executor;
    nhope::AOContext aoCtx(executor);

    auto conn = nhope::PushbackReader::create(aoCtx, nhope::StringReader::create(aoCtx, rawRequest));
    receiveRequest(aoCtx, *conn)
      .then([](auto req) {
          EXPECT_EQ(req.method, "GET");
          EXPECT_EQ(req.uri.toString(), "/path");
          EXPECT_EQ(req.headers, Headers({
                                   {"Content-Length", "10"},
                                 }));

          return nhope::readAll(std::move(req.body));
      })
      .then([](auto bodyData) {
          EXPECT_EQ(asString(bodyData), "1234567890");
      })
      .get();
}

TEST(ReceiveRequest, BadRequest)   // NOLINT
{
    constexpr auto badRequest = "GET /path Invalid HTTP/1.1\r\n";

    nhope::ThreadExecutor executor;
    nhope::AOContext aoCtx(executor);
    auto conn = nhope::PushbackReader::create(aoCtx, nhope::StringReader::create(aoCtx, badRequest));

    auto future = receiveRequest(aoCtx, *conn);

    EXPECT_THROW(future.get(), HttpError);   // NOLINT
}

TEST(ReceiveRequest, IncompleteRequest)   // NOLINT
{
    constexpr auto incompleteRequest = "GET /path HTTP/1.1\r";

    nhope::ThreadExecutor executor;
    nhope::AOContext aoCtx(executor);
    auto conn = nhope::PushbackReader::create(aoCtx, nhope::StringReader::create(aoCtx, incompleteRequest));

    auto future = receiveRequest(aoCtx, *conn);

    EXPECT_THROW(future.get(), HttpError);   // NOLINT
}

TEST(ReceiveRequest, IOError)   // NOLINT
{
    nhope::ThreadExecutor executor;
    nhope::AOContext aoCtx(executor);

    auto conn = nhope::PushbackReader::create(
      aoCtx,                                                                           //
      nhope::concat(aoCtx, nhope::StringReader::create(aoCtx, "GET /path HTTP/1.1"),   // Begining of the request header
                    IOErrorReader::create(aoCtx))                                      // IOError
    );

    auto future = receiveRequest(aoCtx, *conn);

    EXPECT_THROW(future.get(), std::system_error);   // NOLINT
}

TEST(ReceiveRequest, BodyReader_IOError)   // NOLINT
{
    nhope::ThreadExecutor executor;
    nhope::AOContext aoCtx(executor);

    auto conn = nhope::PushbackReader::create(
      aoCtx,                                                                      //
      nhope::concat(aoCtx,                                                        //
                    nhope::StringReader::create(aoCtx, "GET /path HTTP/1.1\r\n"   // Begining of the request header
                                                       "Content-Length: 10\r\n"   //
                                                       "\r\n"                     //
                                                       "123456"),                 //
                    IOErrorReader::create(aoCtx))                                 // IOError
    );

    auto future = receiveRequest(aoCtx, *conn).then([](auto req) {
        return nhope::readAll(std::move(req.body));
    });

    EXPECT_THROW(future.get(), std::system_error);   // NOLINT
}

TEST(ReceiveRequest, Cancel)   // NOLINT
{
    nhope::ThreadExecutor executor;
    nhope::AOContext aoCtx(executor);

    auto conn = nhope::PushbackReader::create(
      aoCtx,                                                                    //
      nhope::concat(aoCtx,                                                      //
                    nhope::StringReader::create(aoCtx, "GET /path HTTP/1.1"),   // Begining of the request header
                    SlowIODevice::create(aoCtx))                                // long reading
    );

    auto future = receiveRequest(aoCtx, *conn);

    EXPECT_FALSE(future.waitFor(100ms));

    aoCtx.close();

    EXPECT_THROW(future.get(), nhope::AsyncOperationWasCancelled);   // NOLINT
}

TEST(ReceiveRequest, BodyReader_Cancel)   // NOLINT
{
    nhope::ThreadExecutor executor;
    nhope::AOContext aoCtx(executor);

    auto conn = nhope::PushbackReader::create(
      aoCtx,                                                                      //
      nhope::concat(aoCtx,                                                        //
                    nhope::StringReader::create(aoCtx, "GET /path HTTP/1.1\r\n"   // Begining of the request header
                                                       "Content-Length: 10\r\n"   //
                                                       "\r\n"                     //
                                                       "123456"),                 //
                    SlowIODevice::create(aoCtx))                                  // long body reading
    );

    auto future = receiveRequest(aoCtx, *conn).then([](auto req) {
        return nhope::readAll(std::move(req.body));
    });

    aoCtx.close();

    EXPECT_THROW(future.get(), nhope::AsyncOperationWasCancelled);   // NOLINT
}

TEST(ReceiveRequest, TwoRequest)   // NOLINT
{
    nhope::ThreadExecutor executor;
    nhope::AOContext aoCtx(executor);

    auto conn = nhope::PushbackReader::create(   //
      aoCtx,                                     //
      nhope::StringReader::create(aoCtx, "GET /first HTTP/1.1\r\n"
                                         "Content-Length: 10\r\n"
                                         "\r\n"
                                         "first-body"
                                         "GET /second HTTP/1.1\r\n"
                                         "Content-Length: 11\r\n"
                                         "\r\n"
                                         "second-body"));

    receiveRequest(aoCtx, *conn)
      .then([](auto req) {
          EXPECT_EQ(req.uri.toString(), "/first");
          return nhope::readAll(std::move(req.body));
      })
      .then([&](auto content) {
          EXPECT_EQ(asString(content), "first-body");

          return receiveRequest(aoCtx, *conn).then([](auto req) {
              EXPECT_EQ(req.uri.toString(), "/second");
              return nhope::readAll(std::move(req.body));
          });
      })
      .then([](auto content) {
          EXPECT_EQ(asString(content), "second-body");
      })
      .get();
}
