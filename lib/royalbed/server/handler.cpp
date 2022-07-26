#include <string>

#include "royalbed/server/detail/handler.h"
#include "nhope/io/string-reader.h"

namespace royalbed::server::detail {

void addContent(RequestContext& ctx, std::string content)
{
    //TODO make other mime-types
    ctx.response.headers.emplace("Content-Type", "application/json");
    ctx.response.headers.emplace("Content-Length", std::to_string(content.size()));
    ctx.response.body = nhope::StringReader::create(ctx.aoCtx, std::move(content));
}

}   // namespace royalbed::server::detail
