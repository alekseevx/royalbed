#include <gtest/gtest.h>
#include <string>
#include "nhope/async/ao-context.h"
#include "nhope/async/executor.h"
#include "nhope/async/thread-executor.h"
#include "nhope/io/io-device.h"
#include "royalbed/common/http-status.h"
#include "royalbed/common/response.h"

using namespace royalbed::common;
using namespace std::literals;

TEST(HttpStatus, message)   // NOLINT
{
    constexpr auto unknownStatus = -1000;
    constexpr auto knownStatuses = std::array{
      HttpStatus::Continue,
      HttpStatus::SwitchingProtocols,
      HttpStatus::Processing,
      HttpStatus::Ok,
      HttpStatus::Created,
      HttpStatus::Accepted,
      HttpStatus::NonAuthoritativeInformation,
      HttpStatus::NoContent,
      HttpStatus::ResetContent,
      HttpStatus::PartialContent,
      HttpStatus::MultiStatus,
      HttpStatus::AlreadyReported,
      HttpStatus::ImUsed,
      HttpStatus::MultipleChoices,
      HttpStatus::MovedPermanently,
      HttpStatus::Found,
      HttpStatus::SeeOther,
      HttpStatus::NotModified,
      HttpStatus::UseProxy,
      HttpStatus::Reserved,
      HttpStatus::TemporaryRedirect,
      HttpStatus::PermanentRedirect,
      HttpStatus::BadRequest,
      HttpStatus::Unauthorized,
      HttpStatus::PaymentRequired,
      HttpStatus::Forbidden,
      HttpStatus::NotFound,
      HttpStatus::MethodNotAllowed,
      HttpStatus::NotAcceptable,
      HttpStatus::ProxyAuthenticationRequired,
      HttpStatus::RequestTimeout,
      HttpStatus::Conflict,
      HttpStatus::Gone,
      HttpStatus::LengthRequired,
      HttpStatus::PreconditionFailed,
      HttpStatus::RequestEntityTooLarge,
      HttpStatus::RequestUriTooLong,
      HttpStatus::UnsupportedMediaType,
      HttpStatus::RequestedRangeNotSatisfiable,
      HttpStatus::ExpectationFailed,
      HttpStatus::UnprocessableEntity,
      HttpStatus::Locked,
      HttpStatus::FailedDependency,
      HttpStatus::UpgradeRequired,
      HttpStatus::PreconditionRequired,
      HttpStatus::TooManyRequests,
      HttpStatus::RequestHeaderFieldsTooLarge,
      HttpStatus::InternalServerError,
      HttpStatus::NotImplemented,
      HttpStatus::BadGateway,
      HttpStatus::ServiceUnavailable,
      HttpStatus::GatewayTimeout,
      HttpStatus::HttpVersionNotSupported,
      HttpStatus::VariantAlsoNegotiates,
      HttpStatus::InsufficientStorage,
      HttpStatus::LoopDetected,
      HttpStatus::NotExtended,
      HttpStatus::NetworkAuthenticationRequired,
    };

    for (const auto knownStatus : knownStatuses) {
        EXPECT_FALSE(HttpStatus::message(knownStatus).empty());
    }

    EXPECT_TRUE(HttpStatus::message(unknownStatus).empty());
}

TEST(HttpStatus, Response)   // NOLINT
{
    nhope::ThreadExecutor e;
    nhope::AOContext ctx(e);
    auto errMsg = "something wrong"sv;
    auto resp = makePlainTextResponse(ctx, HttpStatus::BadRequest, errMsg);
    auto raw = nhope::readAll(*resp.body).get();
    std::string body(raw.begin(), raw.end());

    EXPECT_EQ(resp.status, HttpStatus::BadRequest);
    EXPECT_EQ(resp.statusMessage, HttpStatus::message(HttpStatus::BadRequest));
    EXPECT_EQ(errMsg, body.data());
    EXPECT_EQ(resp.headers["Content-Type"], "text/plain; charset=utf-8");
    EXPECT_EQ(resp.headers["Content-Length"], std::to_string(errMsg.size()));
}
