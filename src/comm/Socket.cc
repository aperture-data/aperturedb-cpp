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

std::pair< int, std::unique_ptr< Socket > > Socket::accept(
    std::vector< std::unique_ptr< Socket > >& listening_sockets)
{
    nfds_t number_fds = listening_sockets.size();
    struct timespec timeout;
    struct pollfd pollfds[number_fds];
    int n = 0;
    std::for_each(listening_sockets.cbegin(), listening_sockets.cend(), [&](auto const& socket) {
        //pollfds[n].fd     = socket->fd();
        //pollfds[n].events = POLLIN;
	VLOG(3) << socket->fd();
        n++;
    });

    timeout.tv_sec   = 1;
    timeout.tv_nsec = 0;

    int poll_res = 0;
    while ((poll_res = ::ppoll(&pollfds[0], number_fds, &timeout, NULL)) == 0) {
	    VLOG(3) << "moo";
        //VLOG(3) << "accept(): no activity for " << timeout << " msec. Going back to listening";
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
