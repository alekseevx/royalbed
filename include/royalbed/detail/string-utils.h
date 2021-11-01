#pragma once

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <string>

namespace royalbed::detail {

inline std::string toLower(std::string str)
{
    std::transform(str.begin(), str.end(), str.begin(), [](char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return str;
}

struct LowercaseHash final
{
    std::size_t operator()(const std::string& key) const
    {
        return std::hash<std::string>()(toLower(key));
    }
};

struct LowercaseLess final
{
    bool operator()(const std::string& a, const std::string& b) const
    {
        return toLower(a) < toLower(b);
    }
};

struct LowercaseEqual final
{
    bool operator()(const std::string& a, const std::string& b) const
    {
        return toLower(a) == toLower(b);
    }
};

template<typename T>
T fromString(const std::string& /*unused*/)
{
    static_assert(!std::is_same_v<T, T>, "string can't be converted to T."
                                         "Please define specialization fromString<T>.");
    return T();
}

template<>
std::string fromString(const std::string& str);

template<>
int fromString(const std::string& str);

template<>
unsigned int fromString(const std::string& str);

template<>
long fromString(const std::string& str);

template<>
unsigned long fromString(const std::string& str);

template<>
long long fromString(const std::string& str);

template<>
unsigned long long fromString(const std::string& str);

}   // namespace royalbed::detail
