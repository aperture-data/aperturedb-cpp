/**
 * @section LICENSE
 *
 * The MIT License
 *
 * @copyright Copyright (c) 2017 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "TLSSocket.h"

#include <cstring>
#include <netdb.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <unistd.h>

#include "ExceptionComm.h"
#include "OpenSSLBio.h"
#include "Variables.h"

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
        throw ExceptionComm(TLSError,
                            "Error accepting connection. SSL Error: " +
                            std::to_string(result));
    }
}

void TLSSocket::connect()
{
    auto result = ::SSL_connect(_ssl);

    if (result < 1) {
        throw ExceptionComm(TLSError);
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
