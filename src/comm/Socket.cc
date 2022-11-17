#include "Socket.h"

std::pair< int, std::unique_ptr< Socket > > Socket::accept(
    std::vector< std::unique_ptr< Socket > >& listening_sockets)
{
    nfds_t number_fds = listening_sockets.size();
    struct timespec timeout;
    struct pollfds[number_fds];
    int n = 0;
    std::for_each(listening_sockets.cbegin(), listening_sockets.cend(), [&](auto socket) {
        pollfds[n].fd     = socket.fd();
        pollfds[n].events = POLLIN;
        n++;
    });

    timeout.tv_sec   = 1;
    timespec.tv_nsec = 0;

    int poll_res = 0;
    while ((poll_res = ::ppoll(&pollfds, number_fds, &timeout, NULL)) == 0) {
        VLOG(3) << "accept(): no activity for " << timeout << " msec. Going back to listening";
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
            return std::make_pair(n, socket);
        }
    }

    THROW_EXCEPTION(ConnectionError, "no socket to accept after poll");
}
