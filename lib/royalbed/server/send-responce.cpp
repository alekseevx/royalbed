#include <string>
#include <string_view>
#include <utility>

#include "nhope/io/io-device.h"
#include "nhope/io/string-reader.h"

#include "royalbed/server/responce.h"
#include "royalbed/server/http-status.h"

#include "royalbed/common/detail/write-headers.h"
#include "royalbed/server/detail/send-responce.h"

namespace royalbed::server::detail {

namespace {
using namespace std::literals;
using namespace royalbed::common::detail;

void writeStartLine(const Responce& responce, std::string& out)
{
    out += "HTTP/1.1 "sv;
    out += std::to_string(responce.status);
    out += ' ';
    if (!responce.statusMessage.empty()) {
        out += responce.statusMessage;
    } else {
        out += HttpStatus::message(responce.status);
    }
    out += "\r\n";
}

nhope::ReaderPtr makeResponceHeaderStream(nhope::AOContext& aoCtx, const Responce& responce)
{
    std::string responceHeader;
    writeStartLine(responce, responceHeader);
    writeHeaders(responce.headers, responceHeader);
    responceHeader += "\r\n"sv;
    return nhope::StringReader::create(aoCtx, std::move(responceHeader));
}

nhope::ReaderPtr makeResponceStream(nhope::AOContext& aoCtx, Responce&& responce)
{
    if (responce.body == nullptr) {
        return makeResponceHeaderStream(aoCtx, responce);
    }

    return nhope::concat(aoCtx,                                       //
                         makeResponceHeaderStream(aoCtx, responce),   //
                         std::move(responce.body));
}

}   // namespace

nhope::Future<std::size_t> sendResponce(nhope::AOContext& aoCtx, Responce&& responce, nhope::Writter& device)
{
    auto responceStream = makeResponceStream(aoCtx, std::move(responce));
    return nhope::copy(*responceStream, device).then([responceStream = std::move(responceStream)](auto n) {
        return n;
    });
}

}   // namespace royalbed::server::detail
