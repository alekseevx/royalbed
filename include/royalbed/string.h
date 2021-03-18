#pragma once

#include <string>

namespace royalbed {

template<typename T>
T fromString(const std::string& str);

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
