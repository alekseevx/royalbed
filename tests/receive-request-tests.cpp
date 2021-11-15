#include <cstdint>
#include <exception>
#include <memory>
#include <string>
#include <system_error>
#include <vector>

#include <gtest/gtest.h>

#include "nhope/async/ao-context-error.h"
#include "nhope/async/ao-context.h"
#include "nhope/async/async-invoke.h"
#include "nhope/async/future.h"
#include "nhope/async/thread-executor.h"
#include "nhope/async/thread-pool-executor.h"
#include "nhope/async/timer.h"
#include "nhope/io/io-device.h"
#include "nhope/io/pushback-reader.h"
#include "nhope/io/string-reader.h"

#include "royalbed/http-error.h"
#include "royalbed/receive-request.h"

namespace {
using namespace std::literals;
using namespace royalbed;
using namespace royalbed::detail;

class IOErrorReader final : public nhope::Reader
{
public:
    explicit IOErrorReader(nhope::AOContext& parent)
      : m_aoCtx(parent)
    {}

    void read(gsl::span<std::uint8_t> /*buf*/, nhope::IOHandler handler) override
    {
        m_aoCtx.exec([handler] {
            const auto errCode = std::make_error_code(std::errc::io_error);
            handler(std::make_exception_ptr(std::system_error(errCode)), {});
        });
    }

    static nhope::ReaderPtr create(nhope::AOContext& parent)
    {
        return std::make_unique<IOErrorReader>(parent);
    }

private:
    nhope::AOContext m_aoCtx;
};

class SlowReader final : public nhope::Reader
{
public:
    explicit SlowReader(nhope::AOContext& parent)
      : m_aoCtx(parent)
    {}
    void read(gsl::span<std::uint8_t> buf, nhope::IOHandler handler) override
    {
        nhope::setTimeout(m_aoCtx, 60min, [handler, n = buf.size()](auto) {
            handler(nullptr, n);
        });
    }

    static nhope::ReaderPtr create(nhope::AOContext& parent)
    {
        return std::make_unique<SlowReader>(parent);
    }

private:
    nhope::AOContext m_aoCtx;
};

std::string toString(const std::vector<std::uint8_t>& bytes)
{
    return {
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
      reinterpret_cast<const char*>(bytes.data()),
      bytes.size(),
    };
}

}   // namespace

TEST(ReceiveRequest, OnlyMethodLine)   // NOLINT
{
    constexpr auto rawRequest = "GET /path?k=v#fragment HTTP/1.1\r\n\r\n";

    nhope::ThreadExecutor executor;
    nhope::AOContext aoCtx(executor);
    auto conn = nhope::PushbackReader::create(aoCtx, nhope::StringReader::create(aoCtx, rawRequest));

    const auto req = nhope::asyncInvoke(aoCtx, [&] {
                         return receiveRequest(aoCtx, *conn);
                     }).get();

    EXPECT_EQ(req->method, "GET");
    EXPECT_EQ(req->uri.toString(), "/path?k=v#fragment");
    EXPECT_TRUE(req->headers.empty());

    const auto body = nhope::asyncInvoke(aoCtx, [&] {
                          return nhope::readAll(*req->body);
                      }).get();

    EXPECT_TRUE(body.empty());
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

    const auto req = nhope::asyncInvoke(aoCtx, [&] {
                         return receiveRequest(aoCtx, *conn);
                     }).get();

    EXPECT_EQ(req->method, "GET");
    EXPECT_EQ(req->uri.toString(), "/path");
    EXPECT_EQ(req->headers, Headers({
                              {"Header1", "Value1"},
                              {"Header2", "Value2"},
                            }));

    const auto body = nhope::asyncInvoke(aoCtx, [&] {
                          return nhope::readAll(*req->body);
                      }).get();

    EXPECT_TRUE(body.empty());
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

    const auto req = nhope::asyncInvoke(aoCtx, [&] {
                         return receiveRequest(aoCtx, *conn);
                     }).get();

    EXPECT_EQ(req->method, "GET");
    EXPECT_EQ(req->uri.toString(), "/path");
    EXPECT_EQ(req->headers, Headers({
                              {"Content-Length", "10"},
                            }));

    const auto content = nhope::asyncInvoke(aoCtx, [&] {
                             return nhope::readAll(*req->body);
                         }).get();
    EXPECT_EQ(toString(content), "1234567890");
}

TEST(ReceiveRequest, BadRequest)   // NOLINT
{
    constexpr auto badRequest = "GET /path Invalid HTTP/1.1\r\n";

    nhope::ThreadExecutor executor;
    nhope::AOContext aoCtx(executor);
    auto conn = nhope::PushbackReader::create(aoCtx, nhope::StringReader::create(aoCtx, badRequest));

    auto future = nhope::asyncInvoke(aoCtx, [&] {
        return receiveRequest(aoCtx, *conn);
    });

    EXPECT_THROW(future.get(), HttpError);   // NOLINT
}

TEST(ReceiveRequest, IncompleteRequest)   // NOLINT
{
    constexpr auto incompleteRequest = "GET /path HTTP/1.1\r";

    nhope::ThreadExecutor executor;
    nhope::AOContext aoCtx(executor);
    auto conn = nhope::PushbackReader::create(aoCtx, nhope::StringReader::create(aoCtx, incompleteRequest));

    auto future = nhope::asyncInvoke(aoCtx, [&] {
        return receiveRequest(aoCtx, *conn);
    });

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

    auto future = nhope::asyncInvoke(aoCtx, [&] {
        return receiveRequest(aoCtx, *conn);
    });

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

    auto req = nhope::asyncInvoke(aoCtx, [&] {
                   return receiveRequest(aoCtx, *conn);
               }).get();

    auto future = nhope::asyncInvoke(aoCtx, [&] {
        return nhope::readAll(*req->body);
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
                    SlowReader::create(aoCtx))                                  // long reading
    );

    auto future = nhope::asyncInvoke(aoCtx, [&] {
        return receiveRequest(aoCtx, *conn);
    });

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
                    SlowReader::create(aoCtx))                                    // long body reading
    );

    auto req = nhope::asyncInvoke(aoCtx, [&] {
                   return receiveRequest(aoCtx, *conn);
               }).get();

    auto future = nhope::asyncInvoke(aoCtx, [&] {
        return nhope::readAll(*req->body);
    });

    EXPECT_FALSE(future.waitFor(100ms));

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

    nhope::makeReadyFuture()
      .then(aoCtx,
            [&] {
                return receiveRequest(aoCtx, *conn).then([](auto req) {
                    EXPECT_EQ(req->uri.toString(), "/first");
                    return nhope::readAll(std::move(req->body)).then([](auto content) {
                        EXPECT_EQ(toString(content), "first-body");
                    });
                });
            })
      .then(aoCtx,
            [&] {
                return receiveRequest(aoCtx, *conn).then([](auto req) {
                    EXPECT_EQ(req->uri.toString(), "/second");
                    return nhope::readAll(std::move(req->body)).then([](auto content) {
                        EXPECT_EQ(toString(content), "second-body");
                    });
                });
            })
      .get();
}
