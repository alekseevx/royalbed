#include <string_view>
#include "write-headers.h"

namespace royalbed::detail {
using namespace std::literals;

void writeHeaders(const Headers& headers, std::string& out)
{
    for (const auto& p : headers) {
        out += p.first;
        out += ": "sv;
        out += p.second;
        out += "\r\n"sv;
    }
}

}   // namespace royalbed::detail
