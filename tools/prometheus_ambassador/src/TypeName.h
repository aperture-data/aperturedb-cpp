/**
 *
 * @copyright Copyright (c) 2022 ApertureData Inc.
 *
 */

#pragma once

#include <string>
#include <cxxabi.h>

// gets the human readable type name
template< typename T >
inline std::string type_name(const T& t) {
    return abi::__cxa_demangle(typeid((t)).name(), nullptr, nullptr, nullptr);
}
