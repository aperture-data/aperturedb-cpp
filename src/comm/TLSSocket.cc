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

TLSSocket::TLSSocket(std::unique_ptr<TCPSocket> tcp_socket, SSL* ssl) :
    _ssl(ssl),
    _tcp_socket(std::move(tcp_socket)) // TODO this member is not used
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
    std::cout << "starting with TLSSocket::accept()" << std::endl;
    errno = 0;

    auto result = ::SSL_accept(_ssl);
    int errno_r = errno;
    ERR_print_errors_fp(stdout);
    if (result < 1) {
        // print ssl error as a string
        char errbuf[256];
        int error = ::SSL_get_error(_ssl, result);
        ::ERR_error_string(errno_r, errbuf);

        std::cout << "TLSSocket::accept() - string " << std::string(errbuf) << std::endl;
        std::cout << "TLSSocket::accept() - error no: " << errno_r << std::endl;
        std::cout << "TLSSocket::accept() - error: " << error << std::endl;
        std::cout << "TLSSocket::accept() - result no: " << result << std::endl;
        THROW_EXCEPTION(TLSError, errno_r, "SSL_accept()", result);
    }
    std::cout << "Done with TLSSocket::accept()" << std::endl;
}

void TLSSocket::connect()
{
    std::cout << "starting with TLSSocket::connect()" << std::endl;
    errno = 0;
    auto result = ::SSL_connect(_ssl);
    int errno_r = errno;

    if (result < 1) {
        THROW_EXCEPTION(TLSError, errno_r, "SSL_connect()", result);
    }
    std::cout << "Done with TLSSocket::connect()" << std::endl;
}

std::unique_ptr<TLSSocket> TLSSocket::create(std::unique_ptr<TCPSocket> tcp_socket,
                                             const std::shared_ptr<SSL_CTX>& ssl_ctx)
{
    std::cout << "starting with TLSSocket::create()" << std::endl;
    auto ssl = SSL_new(ssl_ctx.get());

    auto bio = BIO_new(comm_openssl_bio());

    BIO_set_fd(bio, tcp_socket->_socket_fd, BIO_NOCLOSE);
    SSL_set_bio(ssl, bio, bio);

    std::cout << "Done with TLSSocket::create()" << std::endl;
    return std::unique_ptr<TLSSocket>(new TLSSocket(std::move(tcp_socket), ssl));
}
