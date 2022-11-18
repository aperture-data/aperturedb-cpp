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
#include "comm/UnixConnection.h"
#include "comm/UnixSocket.h"
#include "comm/TLS.h"
#include "comm/TLSConnection.h"
#include "comm/Variables.h"

using namespace comm;

class comm::SSLContextMap
{
   public:
    std::unordered_map< int, OpenSSLPointer< SSL_CTX > > map;
};

ConnServerConfigList simpleTCPConfiguration( int port, ConnServerConfig config )
{
	return createConnList( wrapConnServerConfig( TCPConnServerConfig( port, config )));
}

ConnServer::ConnServer(ConnServerConfigList&& config)
    : _configs(std::move(config))
    , _id_to_listening_socket_map()
    , _open_ssl_initializer(OpenSSLInitializer::instance())
    //, _id_to_ssl_ctx_map()
    , _ssl_ctx_map()
    , _listening_sockets()
{
    if (config.size() == 0) {
        THROW_EXCEPTION(ServerConfigError, "A minimum of 1 config is required");
    }
    for (int i = 0; i < _configs.size(); i++) {
        _configs[i]->id = i;
        configure_individual(i);
    }
}

void ConnServer::configure_individual(int id)
{
    auto _config  = _configs.at(id).get();
    auto _ssl_ctx = create_server_context();

    set_default_verify_paths(_ssl_ctx.get());

    std::unique_ptr< Socket > listening_socket;

    if (_config->auto_generate_certificate) {
        auto certificates = generate_certificate();

        ::set_tls_private_key(_ssl_ctx.get(), certificates.private_key);

        ::set_tls_certificate(_ssl_ctx.get(), certificates.cert);
    } else {
        if (!_config->ca_certificate.empty()) {
            ::set_ca_certificate(_ssl_ctx.get(), _config->ca_certificate);
        }

        if (!_config->tls_certificate.empty()) {
            ::set_tls_certificate(_ssl_ctx.get(), _config->tls_certificate);
        }

        if (!_config->tls_private_key.empty()) {
            ::set_tls_private_key(_ssl_ctx.get(), _config->tls_private_key);
        }
    }

    auto tcp_config = dynamic_cast< TCPConnServerConfig* >(_config);

    if (tcp_config != nullptr) {
        if (tcp_config->_port <= 0 ||
            static_cast< unsigned >(tcp_config->_port) > MAX_PORT_NUMBER) {
            THROW_EXCEPTION(PortError);
        }

        // Create a TCP/IP socket
        auto tcp_listening_socket = TCPSocket::create();

        if (!tcp_listening_socket->set_boolean_option(SOL_SOCKET, SO_REUSEADDR, true)) {
            THROW_EXCEPTION(SocketFail, "Unable to create reusable socket");
        }

        if (!tcp_listening_socket->set_boolean_option(IPPROTO_TCP, TCP_NODELAY, true)) {
            THROW_EXCEPTION(SocketFail, "Unable to turn Nagle's off");
        }

        if (!tcp_listening_socket->set_boolean_option(IPPROTO_TCP, TCP_QUICKACK, true)) {
            THROW_EXCEPTION(SocketFail, "Unable to turn quick ack on");
        }
        struct timeval tv = {MAX_RECV_TIMEOUT_SECS, 0};
        if (!tcp_listening_socket->set_timeval_option(SOL_SOCKET, SO_RCVTIMEO, tv)) {
            THROW_EXCEPTION(SocketFail, "Unable to set receive timeout");
        }

        if (!tcp_listening_socket->bind(tcp_config->_port)) {
            THROW_EXCEPTION(BindFail);
        }

        // listening_socket = tcp_listening_socket.get();

    } else {
        auto unix_config           = dynamic_cast< UnixConnServerConfig* >(_config);
        auto unix_listening_socket = UnixSocket::create();

        if (!unix_listening_socket->bind(unix_config->_path)) {
            THROW_EXCEPTION(BindFail);
        }
        listening_socket = std::unique_ptr< Socket >(unix_listening_socket.release());  //.get();
    }

    // Mark socket as pasive
    if (!listening_socket->listen()) {
        THROW_EXCEPTION(ListentFail);
    }
    _ssl_ctx_map->map[id] = std::move(_ssl_ctx);
}

ConnServer::~ConnServer() = default;

// c contains a TCPConnection, unencrypted, connection to a client.
// The ConnServer will implement the protocol negotiation.
// This right now is a simple handshake, design for ApertureDB Server use-case.
// This protocol can be a virtual method in the future to support arbitrary protocols.
std::unique_ptr< Connection > ConnServer::negotiate_protocol(std::shared_ptr< Connection > conn)
{
    // auto tcp_connection = std::static_pointer_cast< TCPConnection >(conn);

    auto config_id = conn->get_config_id();
    auto response = conn->recv_message();
    auto config = _configs[config_id].get();

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
        server_hello_message.protocol = client_hello_message->protocol & config->allowed_protocols;
    }

    conn->send_message(reinterpret_cast< uint8_t* >(&server_hello_message),
                       sizeof(server_hello_message));

    if (server_hello_message.version == 0) {
        THROW_EXCEPTION(ProtocolError, "Protocol version mismatch");
    }

    auto tcp_conn = dynamic_cast< TCPConnection* >(conn.get());
    if ((server_hello_message.protocol & Protocol::TCP) == Protocol::TCP && tcp_conn != nullptr) {
        auto tcp_socket = tcp_conn->release_socket();
        if ((server_hello_message.protocol & Protocol::TLS) == Protocol::TLS) {
            auto tls_socket =
                TLSSocket::create(std::move(tcp_socket), *_ssl_ctx_map->map[config->id].get());

            tls_socket->accept();
            return std::make_unique< TLSConnection >(std::move(tls_socket),config->id, config->metrics);
        } else {
            // Nothing to do, already using TCP
            return std::make_unique< TCPConnection >(std::move(tcp_socket),config->id, config->metrics);
        }
    } else if ((server_hello_message.protocol & Protocol::UNIX) == Protocol::UNIX) {
        auto unix_conn = dynamic_cast< UnixConnection* >(conn.get());

        if (unix_conn == nullptr) {
            THROW_EXCEPTION(ProtocolError, "Unsupported Connection in protocol negotiation");
        }
        auto unix_socket = unix_conn->release_socket();
        // Nothing to do, already using TCP
        // return tcp_connection;
        return std::make_unique< UnixConnection >(std::move(unix_socket),config->id, config->metrics);
    } else {
        THROW_EXCEPTION(ProtocolError, "Protocol negotiation failed");
    }
}

std::unique_ptr< Connection > ConnServer::accept()
{
    auto connected_socket_pair = Socket::accept(_listening_sockets);
    auto connected_socket      = std::move(connected_socket_pair.second).release();
    auto config_id = connected_socket_pair.first;  // TCPSocket::accept(_listening_socket);
    auto& config   = _configs[config_id];

    auto tcp_socket = dynamic_cast< TCPSocket* >(connected_socket);

    std::unique_ptr< Connection > connection;
    if (tcp_socket != nullptr) {
        // make tcp or unix based on config type
        connection = std::unique_ptr< Connection >(
            new TCPConnection(std::unique_ptr< TCPSocket >(tcp_socket),config_id, config->metrics));
    } else {
        auto unix_socket = dynamic_cast< UnixSocket* >(connected_socket);
        if (unix_socket != nullptr) {
            connection = std::unique_ptr< Connection >(nullptr);
        } else {
            delete connected_socket;
        }
    }

    return connection;
}
