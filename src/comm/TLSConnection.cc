/**
 * @copyright Copyright (c) 2021 ApertureData Inc.
 */

#include "comm/TLSConnection.h"

#include <assert.h>
#include <cstdlib>
#include <errno.h>
#include <netdb.h>
#include <string>
#include <unistd.h>

#include "util/gcc_util.h"
DISABLE_WARNING(effc++)
DISABLE_WARNING(suggest-override)
#include <glog/logging.h>
ENABLE_WARNING(suggest-override)
ENABLE_WARNING(effc++)

#include "comm/Exception.h"

using namespace comm;

TLSConnection::TLSConnection(std::unique_ptr< TLSSocket > tls_socket, ConnMetrics* metrics)
    : Connection(metrics), _tls_socket(std::move(tls_socket))
{
}

static long msec_diff(const struct timespec& a, const struct timespec& b)
{
    return (a.tv_sec - b.tv_sec) * 1000 + (a.tv_nsec - b.tv_nsec) / 1000000;
}

size_t TLSConnection::read(uint8_t* buffer, size_t length)
{
again:
    timespec t1;
    clock_gettime(CLOCK_REALTIME_COARSE, &t1);

    errno       = 0;
    auto count  = SSL_read(_tls_socket->_ssl, buffer, length);
    int errno_r = errno;

    if (count <= 0) {
        auto error = SSL_get_error(_tls_socket->_ssl, count);

        if (error == SSL_ERROR_ZERO_RETURN) {
            THROW_EXCEPTION(ConnectionShutDown,  // FIXME -- misleading 'ConnectionShutDown'
                            errno_r,
                            "SSL_read()",
                            error);  // Peer closed conn for writing, so no more reads
        } else if (error == SSL_ERROR_WANT_READ) {
            if (errno_r == EAGAIN) {
                timespec t2;
                clock_gettime(CLOCK_REALTIME_COARSE, &t2);
                auto dt = msec_diff(t2, t1);
                VLOG(3) << "SSL_read(): no activity for " << dt << " msec. Going back to reading";
                goto again;
            } else {
                // This error is recoverable. Consider handling it.
                THROW_EXCEPTION(ReadFail, errno_r, "SSL_read()", error);
            }
        } else if (error == SSL_ERROR_SYSCALL || error == SSL_ERROR_SSL) {
            THROW_EXCEPTION(ConnectionShutDown, errno_r, "SSL_read()", error);
            // FIXME: *we* must close the conn
        } else {
            THROW_EXCEPTION(ReadFail, errno_r, "SSL_read()", error);
        }
    }

    return static_cast< size_t >(count);
}

size_t TLSConnection::write(const uint8_t* buffer, size_t length)
{
    errno       = 0;
    auto count  = SSL_write(_tls_socket->_ssl, buffer, static_cast< int >(length));
    int errno_r = errno;
    if (count <= 0) {
        auto error = SSL_get_error(_tls_socket->_ssl, count);
        THROW_EXCEPTION(WriteFail, errno_r, "SSL_write()", error);
    }

    return static_cast< size_t >(count);
}

std::string TLSConnection::get_source() const { return _tls_socket->print_source(); }

short TLSConnection::get_source_family() const { return _tls_socket->source_family(); }

std::string TLSConnection::get_encryption() const { return "tls"; }

bool TLSConnection::is_open() { return _tls_socket->_tcp_socket->is_open(); }
