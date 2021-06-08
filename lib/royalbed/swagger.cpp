#include <string>

#include <cmrc/cmrc.hpp>
#include <corvusoft/restbed/status_code.hpp>

#include <royalbed/swagger.h>
#include <royalbed/static-files.h>

CMRC_DECLARE(royalbed::swagger);

namespace royalbed {

void swagger(Router& router, const cmrc::file& swaggerApiFile)
{
    router.use("/swagger", staticFiles(cmrc::royalbed::swagger::get_filesystem()));
    router.get("/swagger/api.json", [swaggerApiFile](const auto& session) {
        session->set_headers({
          {"Content-Length", std::to_string(swaggerApiFile.size())},
          {"Content-Type", std::string("api.json")},
        });
        session->close(restbed::OK, restbed::Bytes{swaggerApiFile.begin(), swaggerApiFile.end()});
    });
}

void swagger(Router& router, const cmrc::embedded_filesystem& fs, std::string_view swaggerApiFilePath)
{
    const auto file = fs.open(std::string(swaggerApiFilePath));
    swagger(router, file);
}

}   // namespace royalbed
