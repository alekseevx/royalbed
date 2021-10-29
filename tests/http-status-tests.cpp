#include <gtest/gtest.h>
#include "royalbed/http-status.h"

using namespace royalbed;

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
