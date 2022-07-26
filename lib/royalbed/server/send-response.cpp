#include <string>
#include <string_view>
#include <utility>

#include "nhope/io/io-device.h"
#include "nhope/io/string-reader.h"

#include "royalbed/server/response.h"
#include "royalbed/server/http-status.h"

#include "royalbed/common/detail/write-headers.h"
#include "royalbed/server/detail/send-response.h"

namespace royalbed::server::detail {

namespace {
using namespace std::literals;
using namespace royalbed::common::detail;

void writeStartLine(const Response& response, std::string& out)
{
    out += "HTTP/1.1 "sv;
    out += std::to_string(response.status);
    out += ' ';
    if (!response.statusMessage.empty()) {
        out += response.statusMessage;
    } else {
        out += HttpStatus::message(response.status);
    }
    out += "\r\n";
}

nhope::ReaderPtr makeResponseHeaderStream(nhope::AOContext& aoCtx, const Response& response)
{
    std::string responseHeader;
    writeStartLine(response, responseHeader);
    writeHeaders(response.headers, responseHeader);
    responseHeader += "\r\n"sv;
    return nhope::StringReader::create(aoCtx, std::move(responseHeader));
}

nhope::ReaderPtr makeResponseStream(nhope::AOContext& aoCtx, Response&& response)
{
    if (response.body == nullptr) {
        return makeResponseHeaderStream(aoCtx, response);
    }

    return nhope::concat(aoCtx,                                       //
                         makeResponseHeaderStream(aoCtx, response),   //
                         std::move(response.body));
}

}   // namespace

nhope::Future<std::size_t> sendResponse(nhope::AOContext& aoCtx, Response&& response, nhope::Writter& device)
{
    auto responseStream = makeResponseStream(aoCtx, std::move(response));
    return nhope::copy(*responseStream, device).then([responseStream = std::move(responseStream)](auto n) {
        return n;
    });
}

}   // namespace royalbed::server::detail
