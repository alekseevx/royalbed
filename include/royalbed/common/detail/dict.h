#pragma once

#include <string>
#include <unordered_map>
#include <utility>

#include "string-utils.h"

namespace royalbed::common::detail {

struct DictKeyCaseSensitive final
{
    using Hash = std::hash<std::string>;
    using Equal = std::equal_to<std::string>;
};

struct DictKeyCaseInsensitive final
{
    using Hash = LowercaseHash;
    using Equal = LowercaseEqual;
};

template<typename KeyTraits = DictKeyCaseSensitive>
using Dict = std::unordered_map<std::string, std::string, typename KeyTraits::Hash, typename KeyTraits::Equal>;

}   // namespace royalbed::common::detail
