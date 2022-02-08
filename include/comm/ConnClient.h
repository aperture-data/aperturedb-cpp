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
#include <string>

#include <openssl/ssl.h>

#include "comm/Address.h"
#include "comm/Connection.h"
#include "comm/Macros.h"
#include "comm/Protocol.h"

namespace comm {

    struct ConnClientConfig
    {
        Protocol allowed_protocols{Protocol::TCP};
        std::string ca_certificate{};
        bool verify_certificate{false};
        ConnMetrics* metrics{nullptr};

        ConnClientConfig() = default;

        ConnClientConfig(Protocol allowed_protocols_,
                         std::string ca_certificate_ = "",
                         bool verify_certificate_ = false,
                         ConnMetrics* metrics_ = nullptr) :
            allowed_protocols(allowed_protocols_),
            ca_certificate(std::move(ca_certificate_)),
            verify_certificate(verify_certificate_),
            metrics(metrics_)
        {
        }

        MOVEABLE_BY_DEFAULT(ConnClientConfig);
        COPYABLE_BY_DEFAULT(ConnClientConfig);
    };

    // Implementation of a client
    class ConnClient final
    {

    public:

        explicit ConnClient(const Address& server_address, ConnClientConfig config = {});
        ~ConnClient();

        MOVEABLE_BY_DEFAULT(ConnClient);
        NOT_COPYABLE(ConnClient);

        std::shared_ptr<Connection> connect();

    private:

        ConnClientConfig _config;
        std::shared_ptr<Connection> _connection;
        Address _server;
        std::shared_ptr<SSL_CTX> _ssl_ctx;
    };

}
