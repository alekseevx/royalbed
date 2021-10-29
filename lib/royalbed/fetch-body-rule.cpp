#include <corvusoft/restbed/request.hpp>
#include <corvusoft/restbed/session.hpp>

#include "royalbed/detail/fetch-body-rule.h"
#include "royalbed/http-status.h"

namespace royalbed::detail {

FetchBodyRule::FetchBodyRule(std::string_view contentType, std::size_t sizeLimit)
  : m_contentType(contentType)
  , m_sizeLimit(sizeLimit)
{}

FetchBodyRule::~FetchBodyRule() = default;

bool FetchBodyRule::condition(std::shared_ptr<restbed::Session> session)
{
    const auto req = session->get_request();
    const auto contentType = req->get_header("Content-Type");
    return contentType == m_contentType;
}

void FetchBodyRule::action(std::shared_ptr<restbed::Session> session,
                           const std::function<void(const std::shared_ptr<restbed::Session>)>& callback)
{
    const auto req = session->get_request();

    if (!req->has_header("Content-Length")) {
        session->close(HttpStatus::LengthRequired);
        return;
    }

    const auto contentLength = req->get_header("Content-Length", std::size_t(0));
    if (contentLength > m_sizeLimit) {
        session->close(HttpStatus::RequestEntityTooLarge);
        return;
    }

    session->fetch(contentLength, [callback](auto session, auto /*body*/) {
        callback(std::move(session));
    });
}

}   // namespace royalbed::detail
