#include <cctype>
#include <charconv>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>

#include "fmt/core.h"

#include "royalbed/detail/string-utils.h"

namespace royalbed::detail {

namespace {

class ToIntegerConvertError final : std::invalid_argument
{
public:
    explicit ToIntegerConvertError(std::string_view str, std::string_view what)
      : std::invalid_argument(fmt::format("Unable convert '{}' to integer: {}", str, what))
    {}
};

std::string_view strip(std::string_view str) noexcept
{
    while (!str.empty() && (std::isspace(str.front()) != 0)) {
        str.remove_prefix(1);
    }

    while (!str.empty() && (std::isspace(str.back()) != 0)) {
        str.remove_suffix(1);
    }

    return str;
}

std::string_view removePlus(std::string_view str)
{
    if (!str.empty() && str.front() == '+') {
        str.remove_prefix(1);
    }
    return str;
}

std::string_view normalization(std::string_view str)
{
    str = strip(str);
    str = removePlus(str);
    return str;
}

template<typename T>
T toInteger(std::string_view str)
{
    static_assert(std::is_integral_v<T>);

    const auto normalized = normalization(str);

    T val{};
    const auto result = std::from_chars(normalized.begin(), normalized.end(), val);
    if (result.ec != std::errc{}) {
        const auto errCode = std::make_error_code(result.ec);
        throw ToIntegerConvertError(str, errCode.message());
    }

    if (result.ptr != normalized.end()) {
        throw ToIntegerConvertError(str, "not integer");
    }

    return val;
}

}   // namespace

template<>
std::string fromString(std::string_view str)
{
    return std::string{str};
}

template<>
short fromString(std::string_view str)
{
    return toInteger<short>(str);
}

template<>
unsigned short fromString(std::string_view str)
{
    return toInteger<unsigned short>(str);
}

template<>
int fromString(std::string_view str)
{
    return toInteger<int>(str);
}

template<>
unsigned int fromString(std::string_view str)
{
    return toInteger<unsigned int>(str);
}

template<>
long fromString(std::string_view str)
{
    return toInteger<long>(str);
}

template<>
unsigned long fromString(std::string_view str)
{
    return toInteger<unsigned long>(str);
}

template<>
long long fromString(std::string_view str)
{
    return toInteger<long long>(str);
}

template<>
unsigned long long fromString(std::string_view str)
{
    return toInteger<unsigned long long>(str);
}

}   // namespace royalbed::detail
