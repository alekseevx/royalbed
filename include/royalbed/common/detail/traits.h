#pragma once

#include <cstdlib>
#include <type_traits>
#include <string_view>

#include <nlohmann/json.hpp>

namespace royalbed::common::detail {

template<typename T>
inline constexpr bool canDeserializeJson = nlohmann::detail::has_from_json<nlohmann::json, T>::value;

template<>
inline constexpr bool canDeserializeJson<void> = false;

template<typename T>
inline constexpr bool canSerializeJson = nlohmann::detail::has_to_json<nlohmann::json, T>::value;

}   // namespace royalbed::common::detail
