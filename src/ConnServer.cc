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

#include <string>
#include <cstring>
#include <unistd.h>
#include <cstdlib>

#include <netdb.h>
#include <netinet/tcp.h>

#include "Connection.h"

using namespace comm;

ConnServer::ConnServer(int port):
    _port(port),
    _socket_fd(-1)
{
    if (_port <= 0 || static_cast<unsigned>(_port) > MAX_PORT_NUMBER) {
        throw ExceptionComm(PortError);
    }

    //create TCP/IP socket
    _socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (_socket_fd < 0) {
        throw ExceptionComm(SocketFail);
    }

    int option = 1; // To set REUSEADDR to true
    int ret = setsockopt(_socket_fd, SOL_SOCKET, SO_REUSEADDR,
                     &option, sizeof(option));
    if (ret < 0) {
        throw ExceptionComm(SocketFail, "Unable to create reusable socket");
    }

    ret = setsockopt(_socket_fd, IPPROTO_TCP, TCP_NODELAY,
                     &option, sizeof(option));
    if (ret < 0) {
        throw ExceptionComm(SocketFail, "Unable to turn Nagle's off");
    }

    ret = setsockopt(_socket_fd, IPPROTO_TCP, TCP_QUICKACK,
                     &option, sizeof(option));
    if (ret < 0) {
        throw ExceptionComm(SocketFail, "Unable to turn quick ack on");
    }

    struct timeval tv = { MAX_RECV_TIMEOUT_SECS, 0 };
    ret = setsockopt(_socket_fd, SOL_SOCKET, SO_RCVTIMEO,
                     &tv, sizeof(tv));
    if (ret < 0) {
        throw ExceptionComm(SocketFail, "Unable to set receive timeout");
    }

    struct sockaddr_in svr_addr;
    memset(&svr_addr, 0, sizeof(svr_addr));
    svr_addr.sin_family = AF_INET;
    svr_addr.sin_addr.s_addr = INADDR_ANY;
    svr_addr.sin_port = htons(_port);

    // bind socket : "assigning a name to a socket"
    ret = ::bind(_socket_fd, reinterpret_cast<const sockaddr*>(&svr_addr), sizeof(svr_addr));
    if (ret < 0) {
        throw ExceptionComm(BindFail);
    }

    //mark socket as pasive
    if (::listen(_socket_fd, MAX_CONN_QUEUE) == -1) {
        throw ExceptionComm(ListentFail);
    }
}

ConnServer::~ConnServer()
{
    if (_socket_fd != -1) {
        ::close(_socket_fd);
        _socket_fd = -1;
    }
}

Connection ConnServer::accept()
{
    struct sockaddr_in clnt_addr;
    socklen_t len = sizeof(clnt_addr); //store size of the address

    // This is where client connects.
    // Server will stall here until incoming connection
    // unless the socket is marked and nonblocking
    int connfd = ::accept(_socket_fd, reinterpret_cast<sockaddr*>(&clnt_addr), &len);

    if (connfd < 0) {
        throw ExceptionComm(ConnectionError);
    }

    return Connection(connfd);
}
