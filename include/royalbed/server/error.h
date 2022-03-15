#pragma once

#include "royalbed/common/http-error.h"

namespace royalbed::server {

using common::HttpError;

class RouterError : public std::runtime_error
{
public:
    explicit RouterError(const std::string& message);
};

}   // namespace royalbed::server
