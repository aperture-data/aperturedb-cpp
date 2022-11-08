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

#include "comm/TCPConnection.h"

#include <assert.h>
#include <cstdlib>
#include <netdb.h>
#include <string>
#include <unistd.h>

#include "comm/Exception.h"
#include "util/gcc_util.h"

using namespace comm;

TCPConnection::TCPConnection(ConnMetrics* metrics) : Connection(metrics), _tcp_socket() {}

TCPConnection::TCPConnection(std::unique_ptr< TCPSocket > tcp_socket, ConnMetrics* metrics)
    : Connection(metrics), _tcp_socket(std::move(tcp_socket))
{
}

size_t TCPConnection::read(uint8_t* buffer, size_t length)
{
    if (!_tcp_socket) {
        THROW_EXCEPTION(SocketFail);
    }

    errno       = 0;
    auto count  = ::recv(_tcp_socket->_socket_fd, buffer, length, MSG_WAITALL);
    int errno_r = errno;

    if (count < 0) {
        DISABLE_WARNING(logical-op)
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            THROW_EXCEPTION(ConnectionShutDown,  // Not, really. Read expired for blocking socket.
                            errno_r,
                            "recv()",
                            0);
        } else {
            THROW_EXCEPTION(ReadFail, errno_r, "recv()", 0);
        }
        ENABLE_WARNING(logical-op)
    }
    // When a stream socket peer has performed an orderly shutdown, the
    // return value will be 0 (the traditional "end-of-file" return).
    else if (count == 0) {
        THROW_EXCEPTION(ConnectionShutDown, "Peer Closed Connection.");
    }

    return static_cast< size_t >(count);
}

std::unique_ptr< TCPSocket > TCPConnection::release_socket() { return std::move(_tcp_socket); }

void TCPConnection::shutdown()
{
    if (_tcp_socket) {
        _tcp_socket->shutdown();
    }
}

size_t TCPConnection::write(const uint8_t* buffer, size_t length)
{
    if (!_tcp_socket) {
        THROW_EXCEPTION(SocketFail);
    }

    // We need MSG_NOSIGNAL so we don't get SIGPIPE, and we can throw.
    auto count = ::send(_tcp_socket->_socket_fd, buffer, length, MSG_NOSIGNAL);

    if (count < 0) {
        THROW_EXCEPTION(WriteFail, "Error sending message.");
    }

    return static_cast< size_t >(count);
}
