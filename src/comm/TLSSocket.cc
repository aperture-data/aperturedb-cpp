/**
 * @copyright Copyright (c) 2021 ApertureData Inc.
 */

#include "comm/TLSSocket.h"
#include <openssl/err.h>

#include <cstring>
#include <netdb.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <unistd.h>

#include "comm/Exception.h"
#include "comm/OpenSSLBio.h"
#include "comm/Variables.h"

using namespace comm;

TLSSocket::TLSSocket(std::unique_ptr< TCPSocket > tcp_socket, SSL* ssl)
    : _ssl(ssl), _tcp_socket(std::move(tcp_socket))
{
}

TLSSocket::~TLSSocket()
{
    if (_ssl) {
        int status = SSL_get_shutdown(_ssl);
        if ((status & (SSL_SENT_SHUTDOWN | SSL_RECEIVED_SHUTDOWN)) == 0) {
            SSL_shutdown(_ssl);
        }
        SSL_free(_ssl);
    }
}

void TLSSocket::accept()
{
    errno       = 0;
    auto result = ::SSL_accept(_ssl);
    int errno_r = errno;
    ERR_print_errors_fp(stdout);
    if (result < 1) {
        THROW_EXCEPTION(TLSError, errno_r, "SSL_accept()", result);
    }
}

void TLSSocket::connect()
{
    errno       = 0;
    auto result = ::SSL_connect(_ssl);
    int errno_r = errno;

    if (result < 1) {
        THROW_EXCEPTION(TLSError, errno_r, "SSL_connect()", result);
    }
}

std::unique_ptr< TLSSocket > TLSSocket::create(std::unique_ptr< TCPSocket > tcp_socket,
                                               const std::shared_ptr< SSL_CTX >& ssl_ctx)
{
    auto ssl = SSL_new(ssl_ctx.get());

    auto bio = BIO_new(comm_openssl_bio());

    BIO_set_fd(bio, tcp_socket->_socket_fd, BIO_NOCLOSE);
    SSL_set_bio(ssl, bio, bio);

    return std::unique_ptr< TLSSocket >(new TLSSocket(std::move(tcp_socket), ssl));
}

std::string TLSSocket::print_source() 
{
	return _tcp_socket->print_source();
}

short TLSSocket::source_family() 
{
	return _tcp_socket->source_family();
}
