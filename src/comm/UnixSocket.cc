/**
 * @copyright Copyright (c) 2021 ApertureData Inc.
 */

#include "comm/UnixSocket.h"

#include <cstring>
#include <netdb.h>
#include <sys/un.h>
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
UnixSocket::UnixSocket(int socket_fd, sockaddr_un address)
    : _socket_fd(socket_fd), _source_family(AF_UNIX), _source(std::move(address))
{
}

UnixSocket::~UnixSocket()
{
    if (_socket_fd != -1) {
        ::close(_socket_fd);
    }
}

static long msec_diff(const struct timespec& a, const struct timespec& b)
{
    return (a.tv_sec - b.tv_sec) * 1000 + (a.tv_nsec - b.tv_nsec) / 1000000;
}

std::unique_ptr< Socket > UnixSocket::accept()
{
    struct sockaddr_un clnt_addr;
    socklen_t len = sizeof(clnt_addr);  // store size of the address

    // This is where client connects.
    // Server will stall here until incoming connection
    // unless the socket is marked as nonblocking
again:
    timespec t1;
    clock_gettime(CLOCK_REALTIME_COARSE, &t1);

    errno                = 0;
    int connected_socket = ::accept(_socket_fd, reinterpret_cast< sockaddr* >(&clnt_addr), &len);

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

    return std::unique_ptr< UnixSocket >(new UnixSocket(connected_socket, std::move(clnt_addr)));
}

#define SUN_PATH_MAX 108
bool UnixSocket::bind(const std::string& path)
{
    if (path.size() + 1 > SUN_PATH_MAX || path.size() == 0) {
        THROW_EXCEPTION(BindFail, "Path size incorrect");
    }

    struct sockaddr_un svr_addr;
    memset(&svr_addr, 0, sizeof(svr_addr));
    svr_addr.sun_family = AF_UNIX;
    strncpy(svr_addr.sun_path, path.c_str(), path.size() + 1);

    // bind socket : "assigning a name to a socket"
    int ret = ::bind(_socket_fd, reinterpret_cast< const sockaddr* >(&svr_addr), sizeof(svr_addr));
    if (ret != 0) {
        char errbuf[256];
        VLOG(3) << "Bind Failed: " << strerror_r(errno, errbuf, 256) << "\n";
    }
    return ret == 0;
    //      return ::bind(
    //         _socket_fd, reinterpret_cast< const sockaddr* >(&svr_addr), sizeof(svr_addr)) == 0;
}

bool UnixSocket::connect(const std::string& path)
{
    if (path.size() > 107 || path.size() == 0) {
        THROW_EXCEPTION(ConnectionError, "Path size incorrect");
    }

    struct sockaddr_un svr_addr;
    memset(&svr_addr, 0, sizeof(svr_addr));
    svr_addr.sun_family = AF_UNIX;
    strncpy(svr_addr.sun_path, path.c_str(), path.size() + 1);

    return ::connect(
               _socket_fd, reinterpret_cast< const sockaddr* >(&svr_addr), sizeof(svr_addr)) == 0;
}

std::unique_ptr< UnixSocket > UnixSocket::create()
{
    int new_socket = ::socket(AF_UNIX, SOCK_STREAM, 0);
    sockaddr_un source;
    memset(&source, 0, sizeof(_source));

    if (new_socket < 0) {
        THROW_EXCEPTION(SocketFail);
    }

    return std::unique_ptr< UnixSocket >(new UnixSocket(new_socket, std::move(source)));
}

bool UnixSocket::listen() { return ::listen(_socket_fd, MAX_CONN_QUEUE) == 0; }

bool UnixSocket::set_boolean_option(int level, int option_name, bool value)
{
    int option = value ? 1 : 0;

    return ::setsockopt(_socket_fd, level, option_name, &option, sizeof(option)) == 0;
}

bool UnixSocket::set_timeval_option(int level, int option_name, timeval value)
{
    return ::setsockopt(_socket_fd, level, option_name, &value, sizeof(value)) == 0;
}

void UnixSocket::shutdown() { ::shutdown(_socket_fd, SHUT_RDWR); }

std::string UnixSocket::print_source()
{
    if (_source_family == AF_UNSPEC) {
        return "";
    } else {
        return std::string(_source.sun_path);
    }
}

short UnixSocket::source_family() { return _source_family; }
