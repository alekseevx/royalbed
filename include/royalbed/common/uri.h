#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace royalbed::common {

class UriParseError final : public std::runtime_error
{
public:
    explicit UriParseError(std::string_view msg);
};

struct Uri final
{
    using Query = std::vector<std::pair<std::string, std::string>>;

    std::string scheme;
    std::string host;
    std::uint16_t port{};
    std::string path;
    Query query;
    std::string fragment;

    [[nodiscard]] bool isRelative() const noexcept;
    [[nodiscard]] std::string toString() const;

    static Uri parseRelative(std::string_view in);
};

enum class UriEscapeMode
{
    Other,
    Query,
    Path,
};

void uriEscape(std::string& out, std::string_view in, UriEscapeMode mode = UriEscapeMode::Other);
std::string uriEscape(std::string_view in, UriEscapeMode mode = UriEscapeMode::Other);

void uriUnescape(std::string& out, std::string_view in, UriEscapeMode mode = UriEscapeMode::Other);
std::string uriUnescape(std::string_view in, UriEscapeMode mode = UriEscapeMode::Other);

}   // namespace royalbed::common
