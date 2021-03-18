#include <stdexcept>
#include <string>

#include <fmt/core.h>

#include <royalbed/string.h>

namespace royalbed {

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
    const auto val = fromString<unsigned long>(str);
    if (val > std::numeric_limits<int>::max()) {
        const auto message = fmt::format("Unable convert '{}' to unsigned int", str);
        throw std::invalid_argument(message);
    }

    return static_cast<unsigned int>(val);
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

}   // namespace royalbed
