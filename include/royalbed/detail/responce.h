#pragma once

#include <memory>
#include <string>

#include "nhope/io/io-device.h"
#include "royalbed/detail/headers.h"
#include "royalbed/http-status.h"

namespace royalbed::detail {

struct Responce;
using ResponcePtr = std::unique_ptr<Responce>;

struct Responce final
{
    int status = HttpStatus::Ok;
    std::string statusMessage;
    Headers headers;
    nhope::ReaderPtr body;

    static ResponcePtr create();
};

}   // namespace royalbed::detail
