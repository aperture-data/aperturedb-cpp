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

#include "TCPSocket.h"

#include <cstring>
#include <netdb.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <unistd.h>

#include "ExceptionComm.h"
#include "Variables.h"

using namespace comm;

TCPSocket::TCPSocket(int socket_fd) :
    _socket_fd(socket_fd)
{
}

TCPSocket::~TCPSocket()
{
    if (_socket_fd != -1) {
        ::close(_socket_fd);
    }
}

std::unique_ptr<TCPSocket> TCPSocket::accept(const std::unique_ptr<TCPSocket>& listening_socket)
{
    struct sockaddr_in clnt_addr;
    socklen_t len = sizeof(clnt_addr); //store size of the address

    // This is where client connects.
    // Server will stall here until incoming connection
    // unless the socket is marked and nonblocking
    int connected_socket = ::accept(listening_socket->_socket_fd, reinterpret_cast<sockaddr*>(&clnt_addr), &len);

    if (connected_socket < 0) {
        throw ExceptionComm(ConnectionError);
    }

    return std::unique_ptr<TCPSocket>(new TCPSocket(connected_socket));
}

bool TCPSocket::bind(int port)
{
    struct sockaddr_in svr_addr;
    memset(&svr_addr, 0, sizeof(svr_addr));
    svr_addr.sin_family = AF_INET;
    svr_addr.sin_addr.s_addr = INADDR_ANY;
    svr_addr.sin_port = htons(port);

    // bind socket : "assigning a name to a socket"
    return ::bind(_socket_fd, reinterpret_cast<const sockaddr*>(&svr_addr), sizeof(svr_addr)) == 0;
}

bool TCPSocket::connect(const Address& address)
{
    struct hostent* server = gethostbyname(address.addr.c_str());

    if (server == NULL) {
        throw ExceptionComm(ServerAddError);
    }

    struct sockaddr_in svr_addr;
    memset(&svr_addr, 0, sizeof(svr_addr));
    svr_addr.sin_family = AF_INET;

    memcpy(&svr_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    svr_addr.sin_port = htons(address.port);

    return ::connect(_socket_fd, reinterpret_cast<const sockaddr*>(&svr_addr), sizeof(svr_addr)) == 0;
}

std::unique_ptr<TCPSocket> TCPSocket::create()
{
    int tcp_socket = ::socket(AF_INET, SOCK_STREAM, 0);

    if (tcp_socket < 0) {
        throw ExceptionComm(SocketFail);
    }

    return std::unique_ptr<TCPSocket>(new TCPSocket(tcp_socket));
}

bool TCPSocket::listen()
{
    return ::listen(_socket_fd, MAX_CONN_QUEUE) == 0;
}

bool TCPSocket::set_boolean_option(int level, int option_name, bool value)
{
    int option = value ? 1 : 0 ;

    return ::setsockopt(_socket_fd, level, option_name, &option, sizeof(option)) == 0;
}

bool TCPSocket::set_timeval_option(int level, int option_name, timeval value)
{
    return ::setsockopt(_socket_fd, level, option_name, &value, sizeof(value)) == 0;
}

void TCPSocket::shutdown()
{
    ::shutdown(_socket_fd, SHUT_RDWR);
}