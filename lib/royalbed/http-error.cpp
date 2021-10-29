#include "royalbed/http-error.h"
#include "royalbed/http-status.h"

namespace royalbed {

HttpError::HttpError(int httpStatus)
  : HttpError::HttpError(httpStatus, HttpStatus::message(httpStatus))
{}

HttpError::HttpError(int httpStatus, std::string_view message)
  : std::runtime_error(std::string(message))
  , m_httpStatus(httpStatus)
{}

[[nodiscard]] int HttpError::httpStatus() const
{
    return m_httpStatus;
}

}   // namespace royalbed
