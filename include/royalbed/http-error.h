#pragma once

#include <stdexcept>

namespace royalbed {

class HttpError final : public std::runtime_error
{
public:
    HttpError(int httpStatus, std::string_view message);

    [[nodiscard]] int httpStatus() const;

private:
    const int m_httpStatus;
};

}   // namespace royalbed
