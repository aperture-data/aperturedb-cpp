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

#include "comm/Exception.h"
#include "util/gcc_util.h"

using namespace comm;

TLSConnection::TLSConnection(ConnMetrics* metrics)
: Connection(metrics)
{
}

TLSConnection::TLSConnection(
    std::unique_ptr<TLSSocket> tls_socket,
    ConnMetrics* metrics
)
: Connection(metrics)
, _tls_socket(std::move(tls_socket))
{
}

size_t TLSConnection::read(uint8_t* buffer, size_t length)
{
    auto count = SSL_read(_tls_socket->_ssl, buffer, length);

    if (count <= 0) {
        auto error = SSL_get_error(_tls_socket->_ssl, count);

        DISABLE_WARNING(logical-op)
        if (error == SSL_ERROR_ZERO_RETURN) {
            THROW_EXCEPTION(ConnectionShutDown, "Closing Connection.");
        }
        else if (error == SSL_ERROR_SYSCALL && (!errno || errno == EAGAIN)) {
            THROW_EXCEPTION(ConnectionShutDown, "Closing Connection.");
        }
        else {
            THROW_EXCEPTION(ReadFail);
        }
        ENABLE_WARNING(logical-op)
    }

    return static_cast<size_t>(count);
}

size_t TLSConnection::write(const uint8_t* buffer, size_t length)
{
    auto count = SSL_write(_tls_socket->_ssl, buffer, static_cast<int>(length));

    if (count <= 0) {
        THROW_EXCEPTION(WriteFail, "Error sending message.");
    }

    return static_cast<size_t>(count);
}
