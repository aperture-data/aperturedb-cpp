/**
 * @copyright Copyright (c) 2021 ApertureData Inc.
 */

#include "comm/TCPSocket.h"

#include <cstring>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "util/gcc_util.h"
DISABLE_WARNING(effc++)
DISABLE_WARNING(suggest-override)
#include <glog/logging.h>
ENABLE_WARNING(suggest-override)
ENABLE_WARNING(effc++)

#include "comm/Exception.h"
#include "comm/Variables.h"

using namespace comm;
TCPSocket::TCPSocket(int socket_fd, const sockaddr_in& address)
    : _socket_fd(socket_fd), _source_family(AF_INET), _source(address)
{
}

TCPSocket::~TCPSocket()
{
    if (_socket_fd != -1) {
        ::close(_socket_fd);
    }
}

static long msec_diff(const struct timespec& a, const struct timespec& b)
{
    return (a.tv_sec - b.tv_sec) * 1000 + (a.tv_nsec - b.tv_nsec) / 1000000;
}

std::unique_ptr< TCPSocket > TCPSocket::accept(const std::unique_ptr< TCPSocket >& listening_socket)
{
    struct sockaddr_in clnt_addr;
    socklen_t len = sizeof(clnt_addr);  // store size of the address

    // This is where client connects.
    // Server will stall here until incoming connection
    // unless the socket is marked as nonblocking
again:
    timespec t1;
    clock_gettime(CLOCK_REALTIME_COARSE, &t1);

    errno = 0;
    int connected_socket =
        ::accept(listening_socket->_socket_fd, reinterpret_cast< sockaddr* >(&clnt_addr), &len);

    int errno_r = errno;
    if (connected_socket < 0) {
        if (errno_r == EAGAIN) {
            timespec t2;
            clock_gettime(CLOCK_REALTIME_COARSE, &t2);
            auto dt = msec_diff(t2, t1);
            VLOG(3) << "accept(): no activity for " << dt << " msec. Going back to listening";
            goto again;
        } else {
            THROW_EXCEPTION(ConnectionError, errno_r, "accept()", 0);
        }
    }
    // MAGICK can be done here

    return std::unique_ptr< TCPSocket >(new TCPSocket(connected_socket, clnt_addr));
}

bool TCPSocket::bind(int port)
{
    struct sockaddr_in svr_addr;
    memset(&svr_addr, 0, sizeof(svr_addr));
    svr_addr.sin_family      = AF_INET;
    svr_addr.sin_addr.s_addr = INADDR_ANY;
    svr_addr.sin_port        = htons(port);

    // bind socket : "assigning a name to a socket"
    return ::bind(_socket_fd, reinterpret_cast< const sockaddr* >(&svr_addr), sizeof(svr_addr)) ==
           0;
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

    return ::connect(
               _socket_fd, reinterpret_cast< const sockaddr* >(&svr_addr), sizeof(svr_addr)) == 0;
}

std::unique_ptr< TCPSocket > TCPSocket::create()
{
    int tcp_socket = ::socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in source;
    memset(&source, 0, sizeof(_source));

    if (tcp_socket < 0) {
        THROW_EXCEPTION(SocketFail);
    }

    return std::unique_ptr< TCPSocket >(new TCPSocket(tcp_socket, source));
}

bool TCPSocket::listen() { return ::listen(_socket_fd, MAX_CONN_QUEUE) == 0; }

bool TCPSocket::set_boolean_option(int level, int option_name, bool value)
{
    int option = value ? 1 : 0;

    return ::setsockopt(_socket_fd, level, option_name, &option, sizeof(option)) == 0;
}

bool TCPSocket::set_timeval_option(int level, int option_name, timeval value)
{
    return ::setsockopt(_socket_fd, level, option_name, &value, sizeof(value)) == 0;
}

void TCPSocket::shutdown() { ::shutdown(_socket_fd, SHUT_RDWR); }

std::string TCPSocket::print_source()
{
    if (_source_family == AF_UNSPEC) {
        return "";
    } else {
        return std::string(inet_ntoa(_source.sin_addr));
    }
}

short TCPSocket::source_family() { return _source_family; }
