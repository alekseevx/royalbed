#include <stdexcept>
#include <string>

#include <fmt/core.h>

#include "royalbed/detail/string-utils.h"

namespace royalbed::detail {

namespace {

template<auto (&convFn)(const std::string& str, size_t*, int)>
auto toNumber(const std::string& str)
{
    constexpr auto base = 10;

    std::size_t n = 0;
    auto val = convFn(str, &n, base);
    if (n < str.size()) {
        const auto message = fmt::format("Unable convert '{}' to number", str);
        throw std::invalid_argument(message);
    }

    return val;
}

}   // namespace

template<>
std::string fromString(const std::string& str)
{
    return str;
}

template<>
int fromString(const std::string& str)
{
    return toNumber<std::stoi>(str);
}

template<>
unsigned int fromString(const std::string& str)
{
    if constexpr (sizeof(int) == sizeof(long)) {
        return static_cast<unsigned int>(fromString<unsigned long>(str));
    } else {
        // sizeof(int) < sizeof(long)

        const auto val = fromString<long>(str);

        if (val > std::numeric_limits<unsigned int>::max()) {
            const auto message = fmt::format("Unable convert '{}' to unsigned int", str);
            throw std::out_of_range(message);
        }

        // a signed number is converted to a unsigned. Need to make sure
        // that there will be no overflow (by analogy with stoul)
        if (val < std::numeric_limits<int>::min()) {
            const auto message = fmt::format("Unable convert '{}' to unsigned int", str);
            throw std::out_of_range(message);
        }

        return static_cast<unsigned int>(val);
    }
}

template<>
long fromString(const std::string& str)
{
    return toNumber<std::stol>(str);
}

template<>
unsigned long fromString(const std::string& str)
{
    return toNumber<std::stoul>(str);
}

template<>
long long fromString(const std::string& str)
{
    return toNumber<std::stoll>(str);
}

template<>
unsigned long long fromString(const std::string& str)
{
    return toNumber<std::stoull>(str);
}

}   // namespace royalbed::detail
