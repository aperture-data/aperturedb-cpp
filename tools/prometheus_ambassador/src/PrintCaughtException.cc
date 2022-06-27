/**
 *
 * @copyright Copyright (c) 2022 ApertureData Inc.
 *
 */

#include "PrintCaughtException.h"
#include <util/TypeName.h>
#include <util/ExceptionUtil.h>
#include <aperturedb/Exception.h>
#include <comm/Exception.h>
#include <sstream>
#include <cstring>

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
