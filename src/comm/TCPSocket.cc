/**
 * @copyright Copyright (c) 2021 ApertureData Inc.
 */

#include "comm/TCPSocket.h"

#include <cstring>
#include <netdb.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <unistd.h>

#include "comm/Exception.h"
#include "comm/Variables.h"

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
        THROW_EXCEPTION(ConnectionError);
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
        THROW_EXCEPTION(ServerAddError);
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
        THROW_EXCEPTION(SocketFail);
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
