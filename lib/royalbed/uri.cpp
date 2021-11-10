#include <array>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <string>
#include <utility>

#include <fmt/format.h>

#include "nhope/utils/hex.h"
#include "royalbed/detail/uri.h"

namespace royalbed::detail {

namespace {
using namespace std::literals;

bool isAllowedSymbol(char ch, UriEscapeMode mode)
{
    if (ch >= '0' && ch <= '9') {
        return true;
    }
    if (ch >= 'A' && ch <= 'Z') {
        return true;
    }
    if (ch >= 'a' && ch <= 'z') {
        return true;
    }
    if (ch == '-' || ch == '_' || ch == '.' || ch == '~') {
        return true;
    }
    if (ch == '/' && mode == UriEscapeMode::Path) {
        return true;
    }
    return false;
}

std::string_view tail(std::string_view in, std::size_t offset)
{
    return offset < in.size() ? in.substr(offset) : std::string_view{};
}

std::string_view parsePath(std::string& out, std::string_view in)
{
    const auto pathEnd = in.find_first_of("?#"sv);
    uriUnescape(out, in.substr(0, pathEnd), UriEscapeMode::Path);
    return tail(in, pathEnd);
}

void appendQueryParam(Uri::Query& out, std::string_view name, std::string_view value = std::string_view{})
{
    if (name.empty()) {
        // name is empty, ignore it
        return;
    }

    try {
        auto& p = out.emplace_back();
        uriUnescape(p.first, name, UriEscapeMode::Query);
        uriUnescape(p.second, value, UriEscapeMode::Query);
    } catch (...) {
        out.pop_back();
        throw;
    }
}

std::string_view parseQueryParam(Uri::Query& out, std::string_view in)
{
    const auto nameEnd = in.find_first_of("=&"sv);
    const auto name = in.substr(0, nameEnd);
    if (nameEnd == std::string_view::npos) {
        appendQueryParam(out, name);
        return std::string_view{};
    }

    in = tail(in, nameEnd + 1);

    const auto valueEnd = in.find_first_of("&#"sv);
    const auto value = in.substr(0, valueEnd);
    appendQueryParam(out, name, value);

    if (valueEnd == std::string_view::npos) {
        return std::string_view{};
    }

    return tail(in, valueEnd + 1);
}

std::string_view parseQuery(Uri::Query& out, std::string_view in)
{
    auto strQuery = in.substr(0, in.find('#'));
    const auto queryLen = strQuery.size();

    while (!strQuery.empty()) {
        strQuery = parseQueryParam(out, strQuery);
    }

    return tail(in, queryLen);
}

void parseFragment(std::string& out, std::string_view in)
{
    uriUnescape(out, in);
}

void parseRelativeUri(Uri& out, std::string_view in)
{
    in = parsePath(out.path, in);
    if (!in.empty() && in[0] == '?') {
        in = parseQuery(out.query, tail(in, 1));
    }

    if (!in.empty() && in[0] == '#') {
        parseFragment(out.fragment, tail(in, 1));
    }
}

}   // namespace

UriParseError::UriParseError(std::string_view msg)
  : std::runtime_error(fmt::format("UriParseError: {}", msg))
{}

bool Uri::isRelative() const noexcept
{
    return this->host.empty();
}

std::string Uri::toString() const
{
    std::string retval;

    if (!this->scheme.empty()) {
        retval = this->scheme;
        retval += ':';
    }

    if (!this->host.empty()) {
        // TODO: Support IPv6
        retval += "//"sv;
        retval += this->host;

        if (this->port != 0) {
            retval += ':';
            retval += std::to_string(this->port);
        }
    }

    if (!this->path.empty()) {
        uriEscape(retval, this->path, UriEscapeMode::Path);
    } else {
        retval += '/';
    }

    if (!this->query.empty()) {
        retval += '?';
        for (const auto& [name, value] : this->query) {
            uriEscape(retval, name, UriEscapeMode::Query);
            retval += '=';
            uriEscape(retval, value, UriEscapeMode::Query);
            retval += '&';
        }
        retval.pop_back();   // Last &
    }

    if (!this->fragment.empty()) {
        retval += '#';
        uriEscape(retval, this->fragment);
    }

    return retval;
}

Uri Uri::parseRelative(std::string_view in)
{
    Uri retval;
    parseRelativeUri(retval, in);
    return retval;
}

void uriEscape(std::string& out, std::string_view in, UriEscapeMode mode)
{
    out.reserve(out.size() + in.size() + 3 * in.size() / 4);
    for (const char ch : in) {
        if (isAllowedSymbol(ch, mode)) {
            out += ch;
        } else if (ch == ' ' && mode == UriEscapeMode::Query) {
            out.push_back('+');
        } else {
            constexpr std::string_view hex = "0123456789abcdef"sv;
            const auto b = static_cast<std::uint8_t>(ch);
            const auto esc = std::array{'%', hex[b >> 4], hex[b & 0x0f]};
            out.append(esc.data(), esc.size());
        }
    }
}

std::string uriEscape(std::string_view in, UriEscapeMode mode)
{
    std::string out;
    uriEscape(out, in, mode);
    return out;
}

void uriUnescape(std::string& out, std::string_view in, UriEscapeMode mode)
{
    try {
        out.reserve(out.size() + in.size());

        for (auto it = in.begin(); it != in.end(); ++it) {
            if (*it == '%') {
                if (std::distance(it, in.end()) < 3) {
                    throw UriParseError("Incomplete hex escape sequence");
                }
                const auto hi = *++it;
                const auto lo = *++it;
                const auto byte = nhope::fromHex(hi, lo);
                out += static_cast<char>(byte);
                continue;
            }

            if (*it == '+' && mode == UriEscapeMode::Query) {
                out += ' ';
                continue;
            }

            if (isAllowedSymbol(*it, mode)) {
                out += *it;
                continue;
            }

            throw UriParseError("Invalid symbol");
        }
    } catch (const nhope::HexParseError& ex) {
        throw UriParseError("Invalid escape sequence");
    }
}

std::string uriUnescape(std::string_view in, UriEscapeMode mode)
{
    std::string out;
    uriUnescape(out, in, mode);
    return out;
}

}   // namespace royalbed::detail
