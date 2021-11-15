/**
 * @copyright Copyright (c) 2021 ApertureData Inc.
 */

#include "comm/TLSSocket.h"

#include <cstring>
#include <netdb.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <unistd.h>

#include "comm/Exception.h"
#include "comm/OpenSSLBio.h"
#include "comm/Variables.h"

using namespace comm;

TLSSocket::TLSSocket(std::unique_ptr<TCPSocket> tcp_socket, SSL* ssl) :
    _ssl(ssl),
    _tcp_socket(std::move(tcp_socket))
{
}

TLSSocket::~TLSSocket()
{
    if (_ssl) {
        if (!(SSL_get_shutdown(_ssl) & SSL_SENT_SHUTDOWN)) {
            SSL_shutdown(_ssl);
        }
        SSL_free(_ssl);
    }
}

void TLSSocket::accept()
{
    auto result = ::SSL_accept(_ssl);

    if (result < 1) {
        THROW_EXCEPTION(TLSError,
                        "Error accepting connection. SSL Error: " +
                        std::to_string(result));
    }
}

void TLSSocket::connect()
{
    auto result = ::SSL_connect(_ssl);

    if (result < 1) {
        THROW_EXCEPTION(TLSError);
    }
}

std::unique_ptr<TLSSocket> TLSSocket::create(std::unique_ptr<TCPSocket> tcp_socket,
                                             SSL_CTX* ssl_ctx)
{
    auto ssl = SSL_new(ssl_ctx);

    auto bio = BIO_new(comm_openssl_bio());

    BIO_set_fd(bio, tcp_socket->_socket_fd, BIO_NOCLOSE);
    SSL_set_bio(ssl, bio, bio);

    return std::unique_ptr<TLSSocket>(new TLSSocket(std::move(tcp_socket), ssl));
}
