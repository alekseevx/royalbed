#include "royalbed/http-status.h"

namespace royalbed {

using namespace std::literals;

std::string_view HttpStatus::message(int status)
{
    switch (status) {
    case Continue:
        return "Continue"sv;
    case SwitchingProtocols:
        return "Switching Protocols"sv;
    case Processing:
        return "Processing"sv;
    case Ok:
        return "OK"sv;
    case Created:
        return "Created"sv;
    case Accepted:
        return "Accepted"sv;
    case NonAuthoritativeInformation:
        return "Non-Authoritative Information"sv;
    case NoContent:
        return "No Content"sv;
    case ResetContent:
        return "Reset Content"sv;
    case PartialContent:
        return "Partial Content"sv;
    case MultiStatus:
        return "Multi-Status"sv;
    case AlreadyReported:
        return "Already Reported"sv;
    case ImUsed:
        return "IM Used"sv;
    case MultipleChoices:
        return "Multiple Choices"sv;
    case MovedPermanently:
        return "Moved Permanently"sv;
    case Found:
        return "Found"sv;
    case SeeOther:
        return "See Other"sv;
    case NotModified:
        return "Not Modified"sv;
    case UseProxy:
        return "Use Proxy"sv;
    case Reserved:
        return "Reserved"sv;
    case TemporaryRedirect:
        return "Temporary Redirect"sv;
    case PermanentRedirect:
        return "Permanent Redirect"sv;
    case BadRequest:
        return "Bad Request"sv;
    case Unauthorized:
        return "Unauthorized"sv;
    case PaymentRequired:
        return "Payment Required"sv;
    case Forbidden:
        return "Forbidden"sv;
    case NotFound:
        return "Not Found"sv;
    case MethodNotAllowed:
        return "Method Not Allowed"sv;
    case NotAcceptable:
        return "Not Acceptable"sv;
    case ProxyAuthenticationRequired:
        return "Proxy Authentication Required"sv;
    case RequestTimeout:
        return "Request Timeout"sv;
    case Conflict:
        return "Conflict"sv;
    case Gone:
        return "Gone"sv;
    case LengthRequired:
        return "Length Required"sv;
    case PreconditionFailed:
        return "Precondition Failed"sv;
    case RequestEntityTooLarge:
        return "Request Entity Too Large"sv;
    case RequestUriTooLong:
        return "Request URI Too Long"sv;
    case UnsupportedMediaType:
        return "Unsupported Media Type"sv;
    case RequestedRangeNotSatisfiable:
        return "Requested Range Not Satisfiable"sv;
    case ExpectationFailed:
        return "Expectation Failed"sv;
    case UnprocessableEntity:
        return "Unprocessable Entity"sv;
    case Locked:
        return "Locked"sv;
    case FailedDependency:
        return "Failed Dependency"sv;
    case UpgradeRequired:
        return "Upgrade Required"sv;
    case PreconditionRequired:
        return "Precondition Required"sv;
    case TooManyRequests:
        return "Too Many Requests"sv;
    case RequestHeaderFieldsTooLarge:
        return "Request Header Fields Too Large"sv;
    case InternalServerError:
        return "Internal Server Error"sv;
    case NotImplemented:
        return "Not Implemented"sv;
    case BadGateway:
        return "Bad Gateway"sv;
    case ServiceUnavailable:
        return "Service Unavailable"sv;
    case GatewayTimeout:
        return "Gateway Timeout"sv;
    case HttpVersionNotSupported:
        return "HTTP Version Not Supported"sv;
    case VariantAlsoNegotiates:
        return "Variant Also Negotiates"sv;
    case InsufficientStorage:
        return "Insufficient Storage"sv;
    case LoopDetected:
        return "Loop Detected"sv;
    case NotExtended:
        return "Not Extended"sv;
    case NetworkAuthenticationRequired:
        return "Network Authentication Required"sv;
    default:
        return ""sv;
    }
}

}   // namespace royalbed
