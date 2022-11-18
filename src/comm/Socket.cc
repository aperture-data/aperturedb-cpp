#include "Socket.h"

#include "util/gcc_util.h"
DISABLE_WARNING(effc++)
DISABLE_WARNING(suggest-override)
#include <glog/logging.h>
ENABLE_WARNING(suggest-override)
ENABLE_WARNING(effc++)
#include "comm/Exception.h"
#include <poll.h>

using namespace comm;

static long msec_diff(const struct timespec& a, const struct timespec& b)
{
    return (a.tv_sec - b.tv_sec) * 1000 + (a.tv_nsec - b.tv_nsec) / 1000000;
}

std::pair< int, std::unique_ptr< Socket > > Socket::accept(
    std::vector< std::unique_ptr< Socket > >& listening_sockets)
{
    nfds_t number_fds = listening_sockets.size();
    struct timespec timeout;
    struct pollfd pollfds[number_fds];
    int n = 0;

    auto config_socket = [&](std::unique_ptr< Socket >& socket) {
        pollfds[n].fd     = socket->fd();
        pollfds[n].events = POLLIN;
        n++;
    };

    std::for_each(listening_sockets.begin(), listening_sockets.end(), config_socket);

    timeout.tv_sec  = 1;
    timeout.tv_nsec = 0;

    timespec t1;
    clock_gettime(CLOCK_REALTIME_COARSE, &t1);

    int poll_res = 0;
    while ((poll_res = ::ppoll(&pollfds[0], number_fds, &timeout, NULL)) == 0) {
        timespec t2;
        clock_gettime(CLOCK_REALTIME_COARSE, &t2);
        auto dt = msec_diff(t2, t1);
        VLOG(3) << "accept(): no activity for " << dt << " msec. Going back to listening";
    }

    // error
    if (poll_res < 0) {
        int errno_r = errno;
        THROW_EXCEPTION(ConnectionError, errno_r, "accept()", 0);
    }
    // accept first
    for (n = 0; n < number_fds; n++) {
        if (pollfds[n].revents & POLLIN) {
            auto socket = listening_sockets[n]->accept();
            return std::make_pair(n, std::move(socket));
        }
    }

    THROW_EXCEPTION(ConnectionError, "no socket to accept after poll");
}
