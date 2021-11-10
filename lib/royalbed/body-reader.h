#pragma once

#include <memory>
#include <span>

#include "3rdparty/llhttp/llhttp.h"
#include "nhope/async/ao-context.h"
#include "nhope/io/pushback-reader.h"

namespace royalbed::detail {

class BodyReader;
using BodyReaderPtr = std::unique_ptr<BodyReader>;

class BodyReader : public nhope::Reader
{
public:
    static BodyReaderPtr create(nhope::AOContextRef& aoCtx, nhope::PushbackReader& device,
                                std::unique_ptr<llhttp_t> httpParser);
};

}   // namespace royalbed::detail
