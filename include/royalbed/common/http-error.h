#pragma once

#include <stdexcept>
#include <string_view>

namespace royalbed::common {

class HttpError final : public std::runtime_error
{
public:
    explicit HttpError(int httpStatus);
    HttpError(int httpStatus, std::string_view message);

    [[nodiscard]] int httpStatus() const;

private:
    const int m_httpStatus;
};

}   // namespace royalbed::common
