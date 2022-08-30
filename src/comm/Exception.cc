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

#include "comm/Exception.h"

#include <stdio.h>
#include <string.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

void print_exception(const comm::Exception& e, FILE* f)
{
    fprintf(f, "[Exception] %s at %s:%d\n", e.name, e.file, e.line);
    if (e.errno_val != 0)
        fprintf(f, "%s: %s\n", e.msg.c_str(), strerror(e.errno_val));
    else if (!e.msg.empty())
        fprintf(f, "%s\n", e.msg.c_str());
}

std::ostream& comm::operator<<(std::ostream& os, const comm::Exception& o)
{
    static const size_t len = 255;
    char buf[len + 1];
    buf[len] = 0;

    os << o.file << ':' << o.line << ':' << " comm::Exception={"
       << "num=" << o.num << ",name='" << o.name << "'";
    if (!o.msg.empty()) {
        os << ",msg='" << o.msg << "'";
    }
    int e = o.errno_val;
    if (e) {
        buf[0] = 0;
        os << ",Error={"
           << "errno=" << e << ",errmsg='" << strerror_r(e, buf, len) << "'" << '}';
    }
    auto s = o.ssl_err_code;
    if (s) {
        const char* err_str = "?";
        if (false) {
#define _O(X)        \
    }                \
    else if (s == X) \
    {                \
        err_str = #X;
            _O(SSL_ERROR_NONE)
            _O(SSL_ERROR_ZERO_RETURN)
            _O(SSL_ERROR_WANT_READ)
            _O(SSL_ERROR_WANT_WRITE)
            _O(SSL_ERROR_WANT_CONNECT)
            _O(SSL_ERROR_WANT_ACCEPT)
            _O(SSL_ERROR_WANT_X509_LOOKUP)
            _O(SSL_ERROR_WANT_ASYNC)
            _O(SSL_ERROR_WANT_ASYNC_JOB)
            _O(SSL_ERROR_WANT_CLIENT_HELLO_CB)
            _O(SSL_ERROR_SYSCALL)
            _O(SSL_ERROR_SSL)
#undef _O
        }
        const char* r = ERR_reason_error_string(s);
        os << ",sslError={"
           << "err=" << err_str << ",reason='" << (r ? r : "") << "'" << '}';
    }
    os << '}';

    return os;
}
