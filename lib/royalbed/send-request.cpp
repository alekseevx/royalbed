#include <cstddef>
#include <string_view>

#include "nhope/io/io-device.h"
#include "nhope/io/string-reader.h"
#include "royalbed/detail/request.h"
#include "send-request.h"

namespace royalbed::detail {

namespace {
using namespace std::literals;

void writePath(const Request& req, std::string& out)
{
    out += req.uri.toString();
}

void writeStartLine(const Request& req, std::string& out)
{
    out += req.method;
    out += ' ';
    writePath(req, out);
    out += " HTTP/1.1\r\n"sv;
}

void writeHeaders(const Request& req, std::string& out)
{
    for (const auto& p : req.headers) {
        out += p.first;
        out += ": "sv;
        out += p.second;
        out += "\r\n"sv;
    }
}

nhope::ReaderPtr makeRequestHeaderStream(nhope::AOContext& aoCtx, const Request& req)
{
    std::string requestHeader;
    writeStartLine(req, requestHeader);
    writeHeaders(req, requestHeader);
    requestHeader += "\r\n"sv;
    return nhope::StringReader::create(aoCtx, std::move(requestHeader));
}

nhope::ReaderPtr makeRequestStream(nhope::AOContext& aoCtx, RequestPtr request)
{
    if (request->body == nullptr) {
        return makeRequestHeaderStream(aoCtx, *request);
    }

    return nhope::concat(aoCtx,                                      //
                         makeRequestHeaderStream(aoCtx, *request),   //
                         std::move(request->body));
}

}   // namespace

nhope::Future<std::size_t> sendRequest(nhope::AOContext& aoCtx, RequestPtr request, nhope::Writter& device)
{
    auto requestStream = makeRequestStream(aoCtx, std::move(request));
    return nhope::copy(*requestStream, device).then([requestStream = std::move(requestStream)](auto n) {
        return n;
    });
}

}   // namespace royalbed::detail
