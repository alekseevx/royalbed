#pragma once

#include <string>
#include <type_traits>

namespace royalbed {

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

}   // namespace royalbed
