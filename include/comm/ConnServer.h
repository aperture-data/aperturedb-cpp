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

#pragma once

#include <memory>

#include <openssl/ssl.h>

#include "comm/Connection.h"
#include "comm/Protocol.h"
#include "comm/TCPSocket.h"

class OpenSSLInitializer;

namespace comm {

    struct ConnServerConfig
    {
        Protocol allowed_protocols{Protocol::TCP};
        bool auto_generate_certificate{true};
        std::string ca_certificate{};
        std::string tls_certificate{};
        std::string tls_private_key{};

        ConnServerConfig() = default;

        ConnServerConfig(Protocol allowed_protocols_,
                         bool auto_generate_certificate_ = true,
                         std::string ca_certificate_ = "",
                         std::string tls_certificate_ = "",
                         std::string tls_private_key_ = "") :
            allowed_protocols(allowed_protocols_),
            auto_generate_certificate(auto_generate_certificate_),
            ca_certificate(std::move(ca_certificate_)),
            tls_certificate(std::move(tls_certificate_)),
            tls_private_key(std::move(tls_private_key_))
        {
        }
    };

    // Implementation of a server
    class ConnServer
    {

    public:

        explicit ConnServer(int port, ConnServerConfig config = {});
        ConnServer(ConnServer&&) = default;
        ConnServer (const ConnServer&) = delete;

        ConnServer& operator=(ConnServer&&) = default;
        ConnServer& operator=(const ConnServer&) = delete;

        std::unique_ptr<Connection> accept();

    private:

        ConnServerConfig _config;
        std::unique_ptr<TCPSocket> _listening_socket;
        OpenSSLInitializer& _open_ssl_initializer;
        int _port; // Server port
        SSL_CTX* _ssl_ctx{nullptr};
    };

};
