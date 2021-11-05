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

#include "TCPConnection.h"

#include <string>
#include <unistd.h>
#include <cstdlib>
#include <assert.h>

#include <netdb.h>

#include "ExceptionComm.h"
#include "gcc_util.h"

using namespace comm;

TCPConnection::TCPConnection() = default;

TCPConnection::TCPConnection(std::unique_ptr<TCPSocket> tcp_socket) :
    _tcp_socket(std::move(tcp_socket))
{
}

size_t TCPConnection::read(uint8_t* buffer, size_t length)
{
    if (!_tcp_socket) {
        throw ExceptionComm(SocketFail);
    }

    auto count = ::recv(_tcp_socket->_socket_fd, buffer, length, MSG_WAITALL);

    if (count < 0) {
        DISABLE_WARNING(logical-op)
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            throw ExceptionComm(ConnectionShutDown, "Connection being cleaned cause of timeout");
        }
        else {
            throw ExceptionComm(ReadFail);
        }
        ENABLE_WARNING(logical-op)
    }
    // When a stream socket peer has performed an orderly shutdown, the
    // return value will be 0 (the traditional "end-of-file" return).
    else if (count == 0) {
        throw ExceptionComm(ConnectionShutDown, "Closing Connection.");
    }

    return static_cast<size_t>(count);
}

std::unique_ptr<TCPSocket> TCPConnection::release_socket()
{
    return std::move(_tcp_socket);
}

void TCPConnection::shutdown()
{
    if (_tcp_socket) {
        _tcp_socket->shutdown();
    }
}

size_t TCPConnection::write(const uint8_t* buffer, size_t length)
{
    if (!_tcp_socket) {
        throw ExceptionComm(SocketFail);
    }

    // We need MSG_NOSIGNAL so we don't get SIGPIPE, and we can throw.
    auto count = ::send(_tcp_socket->_socket_fd, buffer, length, MSG_NOSIGNAL);

    if (count < 0) {
        throw ExceptionComm(WriteFail, "Error sending message.");
    }

    return static_cast<size_t>(count);
}