/**
 *
 * @copyright Copyright (c) 2022 ApertureData Inc.
 *
 */

#pragma once

#include <util/TypeName.h>
#include <sstream>
#include <cstring>

template<typename EX>
std::string print_aperture_exception(const EX& e) {
    std::ostringstream oss;
    oss << type_name(e) << " " << e.name << " (" << e.file << ":" << e.line << ")";
    if (e.errno_val != 0)
        oss << " [code=" << strerror(e.errno_val) << "]";
    if (!e.msg.empty())
        oss << " " << e.msg;
    return oss.str();
}
