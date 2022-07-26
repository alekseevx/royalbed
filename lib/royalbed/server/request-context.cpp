#include "royalbed/server/request-context.h"

#include "nhope/io/string-reader.h"

namespace royalbed::server {

void RequestContext::makePlainTextResponse(int status, std::string_view msg)
{
    response.status = status;
    response.statusMessage = std::string(HttpStatus::message(status));
    response.headers = {
      {"Content-Type", "text/plain; charset=utf-8"},
      {"Content-Length", std::to_string(msg.size())},
    };
    response.body = nhope::StringReader::create(this->aoCtx, msg.data());
};

}   // namespace royalbed::server
