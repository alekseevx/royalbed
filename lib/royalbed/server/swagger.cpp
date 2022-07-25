#include <string>
#include <unordered_map>

#include <cmrc/cmrc.hpp>

#include "nhope/async/future.h"
#include "nhope/io/string-reader.h"

#include "royalbed/common/mime-type.h"
#include "royalbed/server/router.h"
#include "royalbed/server/static-files.h"
#include "royalbed/server/swagger.h"

CMRC_DECLARE(royalbed::swagger);

namespace {

using namespace std::literals;

}   // namespace

namespace royalbed::server {

void swagger(Router& router, const cmrc::embedded_filesystem& fs, std::string_view openApiFilePath)
{
    router.get("/swagger/doc-api"sv, [mimeType = common::mimeTypeForFileName(openApiFilePath),
                                      file = fs.open(std::string(openApiFilePath))](RequestContext& ctx) {
        ctx.response.body = nhope::StringReader::create(ctx.aoCtx, {file.begin(), file.end()});
        ctx.response.headers["Content-Length"] = std::to_string(file.size());
        ctx.response.headers["Content-Type"] = mimeType;
        return nhope::makeReadyFuture();
    });

    router.use("/"sv, staticFiles(cmrc::royalbed::swagger::get_filesystem()));
}

}   // namespace royalbed::server
