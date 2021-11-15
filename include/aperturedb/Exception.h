/**
 * @section LICENSE
 *
 * The MIT License
 *
 * @copyright Copyright (c) 2017 Intel Corporation
 * @copyright Copyright (c) 2021 ApertureData Inc
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#pragma once

#include <string>

namespace VDMS {

    enum ExceptionType {
        FATAL_Internal_Error,

        WriteFail,    // For write/send failure
        ReadFail,     // For read/recv failure
        BindFail,     // Fail to bind a port
        SocketFail,
        ListentFail,

        ServerAddError,
        PortError,
        ConnectionError,
        ConnectionShutDown,

        ProtocolError,
        TLSError,

        AuthenticationError,

        InvalidMessageSize,
        Undefined = 100,// Any undefined error
    };

    struct Exception {
        // Which exception
        const int num;            // Exception number
        const char *const name;   // Exception name

        // Additional information
        const std::string msg;
        const int errno_val;

        // Where it was thrown
        const char * const file;   // Source file name
        const int line;           // Source line number

        Exception(int exc, const char *exc_name, const char *f, int l)
            : num(exc), name(exc_name),
              msg(), errno_val(0),
              file(f), line(l)
        {}

        Exception(int exc, const char *exc_name,
                  const std::string &m,
                  const char *f, int l)
            : num(exc), name(exc_name),
              msg(m), errno_val(0),
              file(f), line(l)
        {}

        Exception(int exc, const char *exc_name,
                  int err, const std::string &m,
                  const char *f, int l)
            : num(exc), name(exc_name),
              msg(m), errno_val(err),
              file(f), line(l)
        {}
        Exception() = delete;
        Exception(const Exception&) = default;
        Exception& operator=(const Exception&) = delete;
    };

};
