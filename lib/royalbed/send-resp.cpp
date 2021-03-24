#include <string>

#include <corvusoft/restbed/status_code.hpp>
#include <royalbed/http-error.h>
#include <royalbed/send-resp.h>

namespace royalbed {

void sendText(restbed::Session& session, std::string_view text)
{
    sendText(session, restbed::OK, text);
}

void sendText(restbed::Session& session, int httpStatus, std::string_view text)
{
    session.close(httpStatus, std::string(text),
                  {
                    {"Content-Type", "text/plain"},
                    {"Content-Length", std::to_string(text.size())},
                  });
}

void sendJson(restbed::Session& session, const nlohmann::json& value)
{
    sendJson(session, restbed::OK, value);
}

void sendJson(restbed::Session& session, int httpStatus, const nlohmann::json& value)
{
    const auto strValue = value.dump();
    session.close(httpStatus, strValue,
                  {
                    {"Content-Type", "application/json"},
                    {"Content-Length", std::to_string(strValue.size())},
                  });
}

void sendError(restbed::Session& session, std::exception_ptr exPtr)
{
    try {
        std::rethrow_exception(std::move(exPtr));
    } catch (const HttpError& ex) {
        sendText(session, ex.httpStatus(), ex.what());
    } catch (const std::exception& ex) {
        sendText(session, restbed::INTERNAL_SERVER_ERROR, ex.what());
    } catch (...) {
        sendText(session, restbed::INTERNAL_SERVER_ERROR, "An unknown exception was caught");
    }
}

}   // namespace royalbed
