#ifndef UTIL_H
#define UTIL_H

#include <utility>

// Backport from C++23
template<class Enum>
constexpr std::underlying_type_t<Enum> to_underlying(Enum e) noexcept
{
    return static_cast<std::underlying_type_t<Enum>>(e);
}

#endif /* UTIL_H */
