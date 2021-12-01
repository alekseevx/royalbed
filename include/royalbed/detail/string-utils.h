#pragma once

#include <algorithm>
#include <cctype>
#include <functional>
#include <string>
#include <string_view>
#include <type_traits>

namespace royalbed::detail {

struct StringEqual final
{
    using is_transparent = std::true_type;

    bool operator()(std::string_view l, std::string_view r) const noexcept
    {
        return l == r;
    }
};

struct StringHash final
{
    using is_transparent = std::true_type;
    auto operator()(std::string_view s) const noexcept
    {
        return std::hash<std::string_view>{}(s);
    }
};

inline std::string toLower(std::string_view str)
{
    std::string result(str.size(), '\0');
    std::transform(str.begin(), str.end(), result.begin(), [](char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return result;
}

struct LowercaseHash final
{
    std::size_t operator()(std::string_view key) const
    {
        return std::hash<std::string>()(toLower(key));
    }
};

struct LowercaseLess final
{
    bool operator()(std::string_view a, std::string_view b) const
    {
        return toLower(a) < toLower(b);
    }
};

struct LowercaseEqual final
{
    bool operator()(std::string_view a, std::string_view b) const
    {
        return toLower(a) == toLower(b);
    }
};

template<typename T>
T fromString(std::string_view /*unused*/)
{
    static_assert(!std::is_same_v<T, T>, "string can't be converted to T."
                                         "Please define specialization fromString<T>.");
    return T();
}

template<>
std::string fromString(std::string_view str);

template<>
short fromString(std::string_view str);

template<>
unsigned short fromString(std::string_view str);

template<>
int fromString(std::string_view str);

template<>
unsigned int fromString(std::string_view str);

template<>
long fromString(std::string_view str);

template<>
unsigned long fromString(std::string_view str);

template<>
long long fromString(std::string_view str);

template<>
unsigned long long fromString(std::string_view str);

}   // namespace royalbed::detail
