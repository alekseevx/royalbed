#include <string>

#include <cmrc/cmrc.hpp>
#include <corvusoft/restbed/status_code.hpp>

#include <royalbed/mime-type.h>
#include <royalbed/swagger.h>
#include <royalbed/static-files.h>

CMRC_DECLARE(royalbed::swagger);

namespace royalbed {

void swagger(Router& router, const cmrc::embedded_filesystem& fs, std::string_view swaggerApiFilePath)
{
    const auto file = fs.open(std::string(swaggerApiFilePath));
    const auto mimeType = std::string(*mimeTypeForFileName(swaggerApiFilePath));

    router.use("/swagger", staticFiles(cmrc::royalbed::swagger::get_filesystem()));
    router.get("/swagger/doc-api", [file, mimeType](const auto& session) {
        session->set_headers({
          {"Content-Length", std::to_string(file.size())},
          {"Content-Type", mimeType},
        });
        session->close(restbed::OK, restbed::Bytes{file.begin(), file.end()});
    });
}

}   // namespace royalbed
