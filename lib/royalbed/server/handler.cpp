#include <string>

#include "royalbed/server/detail/handler.h"
#include "nhope/io/string-reader.h"

namespace royalbed::server::detail {

void addContent(RequestContext& ctx, std::string content)
{
    ctx.responce.headers.emplace("Content-Type", "application/json");
    ctx.responce.headers.emplace("Content-Length", std::to_string(content.size()));
    ctx.responce.body = nhope::StringReader::create(ctx.aoCtx, std::move(content));
}

}   // namespace royalbed::server::detail
