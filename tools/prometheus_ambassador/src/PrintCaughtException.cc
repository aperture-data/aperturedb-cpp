/**
 *
 * @copyright Copyright (c) 2022 ApertureData Inc.
 *
 */

#include "PrintCaughtException.h"
#include <util/TypeName.h>
#include <aperturedb/Exception.h>
#include <comm/Exception.h>
#include <sstream>
#include <cstring>

namespace {
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
}

std::string print_caught_exception() {
    try {
        throw;
    }
    catch (const VDMS::Exception& e) {
        return print_aperture_exception(e);
    }
    catch (const comm::Exception& e) {
        return print_aperture_exception(e);
    }
    catch (const std::exception& e) {
        return std::string("[") + type_name(e) + "] " + e.what();
    }
    catch (...) {
        return "<unknown exception type>";
    }
}
