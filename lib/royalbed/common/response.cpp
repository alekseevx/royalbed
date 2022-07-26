#include "nhope/async/ao-context.h"
#include "royalbed/common/response.h"

#include "nhope/io/string-reader.h"

namespace royalbed::common {

Response makePlainTextResponse(nhope::AOContext& ctx, int status, std::string_view msg)
{
    return {
      .status = status,
      .statusMessage = std::string(HttpStatus::message(status)),
      .headers =
        {
          {"Content-Type", "text/plain; charset=utf-8"},
          {"Content-Length", std::to_string(msg.size())},
        },
      .body = nhope::StringReader::create(ctx, msg.data()),
    };
};

}   // namespace royalbed::common
