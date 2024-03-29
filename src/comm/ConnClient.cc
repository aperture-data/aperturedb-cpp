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

#include "comm/ConnClient.h"

#include <string>
#include <cstring>
#include <unistd.h>
#include <cstdlib>

#include <netdb.h>
#include <netinet/tcp.h>

#include "comm/Exception.h"
#include "comm/HelloMessage.h"
#include "comm/TCPConnection.h"
#include "comm/TLS.h"
#include "comm/TLSConnection.h"
#include "comm/Variables.h"

using namespace comm;

ConnClient::ConnClient(const Address& server_address, ConnClientConfig config)
    : _config(std::move(config))
    , _connection()
    , _server(std::move(server_address))
    , _ssl_ctx(create_client_context())
{
    set_default_verify_paths(_ssl_ctx.get());

    if (config.verify_certificate) {
        SSL_CTX_set_verify(
            _ssl_ctx.get(), SSL_VERIFY_PEER, ::SSL_CTX_get_verify_callback(_ssl_ctx.get()));
    }

    if (!_config.ca_certificate.empty()) {
        ::set_ca_certificate(_ssl_ctx.get(), _config.ca_certificate);
    }
}

ConnClient::~ConnClient() = default;

std::shared_ptr< Connection > ConnClient::connect()
{
    if (!_connection) {
        if (_server.port <= 0 || static_cast< unsigned >(_server.port) > MAX_PORT_NUMBER) {
            THROW_EXCEPTION(PortError);
        }

        // Create a TCP/IP socket
        auto tcp_socket = TCPSocket::create();

        if (!tcp_socket->set_boolean_option(SOL_SOCKET, SO_REUSEADDR, true)) {
            THROW_EXCEPTION(SocketFail, "Unable to bind client socket");
        }

        if (!tcp_socket->set_boolean_option(IPPROTO_TCP, TCP_NODELAY, true)) {
            THROW_EXCEPTION(SocketFail, "Unable to turn Nagle's off");
        }

        if (!tcp_socket->set_boolean_option(IPPROTO_TCP, TCP_QUICKACK, true)) {
            THROW_EXCEPTION(SocketFail, "Unable to turn quick ack on");
        }

        if (!tcp_socket->connect(_server)) {
            THROW_EXCEPTION(ConnectionError);
        }

        auto tcp_connection = std::unique_ptr< TCPConnection >(
            new TCPConnection(std::move(tcp_socket), _config.metrics));

        HelloMessage client_hello_message;

        client_hello_message.version  = PROTOCOL_VERSION;
        client_hello_message.protocol = _config.allowed_protocols;

        tcp_connection->send_message(reinterpret_cast< uint8_t* >(&client_hello_message),
                                     sizeof(client_hello_message));

        auto response = tcp_connection->recv_message();

        if (response.length() != sizeof(HelloMessage)) {
            THROW_EXCEPTION(InvalidMessageSize);
        }

        auto server_hello_message = reinterpret_cast< const HelloMessage* >(response.data());

        if (server_hello_message->version == 0) {
            THROW_EXCEPTION(ProtocolError, "Protocol version mismatch");
        }

        if (server_hello_message->protocol == Protocol::None) {
            THROW_EXCEPTION(ProtocolError, "Server rejected protocol");
        } else if ((server_hello_message->protocol & Protocol::TLS) == Protocol::TLS) {
            tcp_socket = tcp_connection->release_socket();

            auto tls_socket = TLSSocket::create(std::move(tcp_socket), _ssl_ctx);

            tls_socket->connect();

            _connection = std::unique_ptr< TLSConnection >(
                new TLSConnection(std::move(tls_socket), _config.metrics));
        } else if ((server_hello_message->protocol & Protocol::TCP) == Protocol::TCP) {
            // Nothing to do, already using TCP
            _connection = std::move(tcp_connection);
        } else {
            THROW_EXCEPTION(ProtocolError, "Protocol negotiation failed");
        }
    }

    return std::static_pointer_cast< Connection >(_connection);
}
