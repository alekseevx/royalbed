#include <string>
#include <string_view>
#include <utility>

#include "nhope/io/io-device.h"
#include "nhope/io/string-reader.h"

#include "royalbed/detail/responce.h"
#include "royalbed/http-status.h"

#include "write-headers.h"
#include "send-responce.h"

namespace royalbed::detail {
using namespace std::literals;

namespace {

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

nhope::ReaderPtr makeResponceStream(nhope::AOContext& aoCtx, ResponcePtr responce)
{
    if (responce->body == nullptr) {
        return makeResponceHeaderStream(aoCtx, *responce);
    }

    return nhope::concat(aoCtx,                                        //
                         makeResponceHeaderStream(aoCtx, *responce),   //
                         std::move(responce->body));
}

}   // namespace

nhope::Future<std::size_t> sendResponce(nhope::AOContext& aoCtx, ResponcePtr responce, nhope::Writter& device)
{
    auto responceStream = makeResponceStream(aoCtx, std::move(responce));
    return nhope::copy(*responceStream, device).then([responceStream = std::move(responceStream)](auto n) {
        return n;
    });
}

}   // namespace royalbed::detail
