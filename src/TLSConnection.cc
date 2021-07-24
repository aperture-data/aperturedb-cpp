/**
 * @section LICENSE
 *
 * The MIT License
 *
 * @copyright Copyright (c) 2017 Intel Corporation
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

#include "TLSConnection.h"

#include <assert.h>
#include <cstdlib>
#include <errno.h>
#include <unistd.h>
#include <string>

#include <netdb.h>

#include "ExceptionComm.h"
#include "gcc_util.h"

using namespace comm;

TLSConnection::TLSConnection() = default;

TLSConnection::TLSConnection(std::unique_ptr<TLSSocket> tls_socket) :
    _tls_socket(std::move(tls_socket))
{
}

size_t TLSConnection::read(uint8_t* buffer, size_t length)
{
    auto count = SSL_read(_tls_socket->_ssl, buffer, length);

    if (count <= 0) {
        auto error = SSL_get_error(_tls_socket->_ssl, count);

        DISABLE_WARNING(logical-op)
        if (error == SSL_ERROR_ZERO_RETURN) {
            throw ExceptionComm(ConnectionShutDown, "Closing Connection.");
        }
        else if (error == SSL_ERROR_SYSCALL && (!errno || errno == EAGAIN)) {
            throw ExceptionComm(ConnectionShutDown, "Closing Connection.");
        }
        else {
            throw ExceptionComm(ReadFail);
        }
        ENABLE_WARNING(logical-op)
    }

    return static_cast<size_t>(count);
}

size_t TLSConnection::write(const uint8_t* buffer, size_t length)
{
    auto count = SSL_write(_tls_socket->_ssl, buffer, static_cast<int>(length));

    if (count <= 0) {
        throw ExceptionComm(WriteFail, "Error sending message.");
    }

    return static_cast<size_t>(count);
}
