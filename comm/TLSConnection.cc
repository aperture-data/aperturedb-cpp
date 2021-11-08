/**
 * @copyright Copyright (c) 2021 ApertureData Inc.
 */

#include "TLSConnection.h"

#include <assert.h>
#include <cstdlib>
#include <errno.h>
#include <unistd.h>
#include <string>

#include <netdb.h>

#include "ExceptionComm.h"
#include "gcc_util.h"

using namespace comm;

TLSConnection::TLSConnection() = default;

TLSConnection::TLSConnection(std::unique_ptr<TLSSocket> tls_socket) :
    _tls_socket(std::move(tls_socket))
{
}

size_t TLSConnection::read(uint8_t* buffer, size_t length)
{
    auto count = SSL_read(_tls_socket->_ssl, buffer, length);

    if (count <= 0) {
        auto error = SSL_get_error(_tls_socket->_ssl, count);

        DISABLE_WARNING(logical-op)
        if (error == SSL_ERROR_ZERO_RETURN) {
            throw ExceptionComm(ConnectionShutDown, "Closing Connection.");
        }
        else if (error == SSL_ERROR_SYSCALL && (!errno || errno == EAGAIN)) {
            throw ExceptionComm(ConnectionShutDown, "Closing Connection.");
        }
        else {
            throw ExceptionComm(ReadFail);
        }
        ENABLE_WARNING(logical-op)
    }

    return static_cast<size_t>(count);
}

size_t TLSConnection::write(const uint8_t* buffer, size_t length)
{
    auto count = SSL_write(_tls_socket->_ssl, buffer, static_cast<int>(length));

    if (count <= 0) {
        throw ExceptionComm(WriteFail, "Error sending message.");
    }

    return static_cast<size_t>(count);
}
