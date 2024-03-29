/**
 * @section LICENSE
 *
 * The MIT License
 *
 * @copyright Copyright (c) 2017 Intel Corporation
 * @copyright Copyright (c) 2021 ApertureData Inc
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

#include "comm/ConnServer.h"

#include <string>
#include <cstring>
#include <unistd.h>
#include <cstdlib>

#include <netdb.h>
#include <netinet/tcp.h>

#include "comm/Connection.h"
#include "comm/Exception.h"
#include "comm/HelloMessage.h"
#include "comm/TCPConnection.h"
#include "comm/TCPSocket.h"
#include "comm/TLS.h"
#include "comm/TLSConnection.h"
#include "comm/Variables.h"

using namespace comm;

ConnServer::ConnServer(int port, ConnServerConfig config)
    : _config(std::move(config))
    , _listening_socket()
    , _open_ssl_initializer(OpenSSLInitializer::instance())
    , _port(port)
    , _ssl_ctx(create_server_context())
{
    set_default_verify_paths(_ssl_ctx.get());

    if (_config.auto_generate_certificate) {
        auto certificates = generate_certificate();

        ::set_tls_private_key(_ssl_ctx.get(), certificates.private_key);

        ::set_tls_certificate(_ssl_ctx.get(), certificates.cert);
    } else {
        if (!_config.ca_certificate.empty()) {
            ::set_ca_certificate(_ssl_ctx.get(), _config.ca_certificate);
        }

        if (!_config.tls_certificate.empty()) {
            ::set_tls_certificate(_ssl_ctx.get(), _config.tls_certificate);
        }

        if (!_config.tls_private_key.empty()) {
            ::set_tls_private_key(_ssl_ctx.get(), _config.tls_private_key);
        }
    }

    if (_port <= 0 || static_cast< unsigned >(_port) > MAX_PORT_NUMBER) {
        THROW_EXCEPTION(PortError);
    }

    // Create a TCP/IP socket
    _listening_socket = TCPSocket::create();

    if (!_listening_socket->set_boolean_option(SOL_SOCKET, SO_REUSEADDR, true)) {
        THROW_EXCEPTION(SocketFail, "Unable to create reusable socket");
    }

    if (!_listening_socket->set_boolean_option(IPPROTO_TCP, TCP_NODELAY, true)) {
        THROW_EXCEPTION(SocketFail, "Unable to turn Nagle's off");
    }

    if (!_listening_socket->set_boolean_option(IPPROTO_TCP, TCP_QUICKACK, true)) {
        THROW_EXCEPTION(SocketFail, "Unable to turn quick ack on");
    }

    struct timeval tv = {MAX_RECV_TIMEOUT_SECS, 0};
    if (!_listening_socket->set_timeval_option(SOL_SOCKET, SO_RCVTIMEO, tv)) {
        THROW_EXCEPTION(SocketFail, "Unable to set receive timeout");
    }

    if (!_listening_socket->bind(_port)) {
        THROW_EXCEPTION(BindFail);
    }

    // Mark socket as pasive
    if (!_listening_socket->listen()) {
        THROW_EXCEPTION(ListentFail);
    }
}

ConnServer::~ConnServer() = default;

// c contains a TCPConnection, unencrypted, connection to a client.
// The ConnServer will implement the protocol negotiation.
// This right now is a simple handshake, design for ApertureDB Server use-case.
// This protocol can be a virtual method in the future to support arbitrary protocols.
std::unique_ptr< Connection > ConnServer::negotiate_protocol(std::shared_ptr< Connection > conn)
{
    auto tcp_connection = std::static_pointer_cast< TCPConnection >(conn);

    auto response = tcp_connection->recv_message();

    if (response.length() != sizeof(HelloMessage)) {
        THROW_EXCEPTION(ProtocolError);
    }

    auto client_hello_message = reinterpret_cast< const HelloMessage* >(response.data());

    HelloMessage server_hello_message;

    if (client_hello_message->version != PROTOCOL_VERSION) {
        server_hello_message.version  = 0;
        server_hello_message.protocol = Protocol::None;
    } else {
        server_hello_message.version  = PROTOCOL_VERSION;
        server_hello_message.protocol = client_hello_message->protocol & _config.allowed_protocols;
    }

    tcp_connection->send_message(reinterpret_cast< uint8_t* >(&server_hello_message),
                                 sizeof(server_hello_message));

    if (server_hello_message.version == 0) {
        THROW_EXCEPTION(ProtocolError, "Protocol version mismatch");
    }

    if ((server_hello_message.protocol & Protocol::TLS) == Protocol::TLS) {
        auto tcp_socket = tcp_connection->release_socket();

        auto tls_socket = TLSSocket::create(std::move(tcp_socket), _ssl_ctx);

        tls_socket->accept();

        return std::make_unique< TLSConnection >(std::move(tls_socket), _config.metrics);
    } else if ((server_hello_message.protocol & Protocol::TCP) == Protocol::TCP) {
        auto tcp_socket = tcp_connection->release_socket();
        // Nothing to do, already using TCP
        // return tcp_connection;
        return std::make_unique< TCPConnection >(std::move(tcp_socket), _config.metrics);
    } else {
        THROW_EXCEPTION(ProtocolError, "Protocol negotiation failed");
    }
}

std::unique_ptr< Connection > ConnServer::accept()
{
    auto connected_socket = TCPSocket::accept(_listening_socket);

    auto tcp_connection =
        std::make_unique< TCPConnection >(std::move(connected_socket), _config.metrics);

    return tcp_connection;
}
