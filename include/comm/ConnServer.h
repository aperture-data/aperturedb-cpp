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
#include <vector>
#include <unordered_map>

#include <openssl/ssl.h>

#include "comm/Connection.h"
#include "util/Macros.h"
#include "comm/Protocol.h"

class OpenSSLInitializer;

namespace comm
{

class TCPSocket;

struct ConnServerConfig {
    Protocol allowed_protocols{Protocol::TCP};
    bool auto_generate_certificate{true};
    std::string ca_certificate{};
    std::string tls_certificate{};
    std::string tls_private_key{};
    ConnMetrics* metrics{nullptr};

    ConnServerConfig()          = default;
    virtual ~ConnServerConfig() = default;

    ConnServerConfig(Protocol allowed_protocols_,
                     bool auto_generate_certificate_ = true,
                     std::string ca_certificate_     = "",
                     std::string tls_certificate_    = "",
                     std::string tls_private_key_    = "",
                     ConnMetrics* metrics_           = nullptr)
        : allowed_protocols(allowed_protocols_)
        , auto_generate_certificate(auto_generate_certificate_)
        , ca_certificate(std::move(ca_certificate_))
        , tls_certificate(std::move(tls_certificate_))
        , tls_private_key(std::move(tls_private_key_))
        , metrics(metrics_)
    {
    }

    MOVEABLE_BY_DEFAULT(ConnServerConfig);
    COPYABLE_BY_DEFAULT(ConnServerConfig);
};

struct TCPConnServerConfig : public ConnServerConfig {
    int _port;
};

struct UnixConnServerConfig : public ConnServerConfig {
    std::string _path{};
};

typedef std::vector< std::unique_ptr< ConnServerConfig > > ConnServerConfigList;

class SSLContextMap;
// Implementation of a server
class ConnServer final
{
   public:
    explicit ConnServer(ConnServerConfigList);
    ~ConnServer();

    MOVEABLE_BY_DEFAULT(ConnServer);
    NOT_COPYABLE(ConnServer);

    std::unique_ptr< Connection > accept();

    std::unique_ptr< Connection > negotiate_protocol(std::shared_ptr< Connection > conn);

   private:
    ConnServerConfigList _configs;
    std::unordered_map< int, std::unique_ptr< TCPSocket > > _id_to_listening_socket_map;
    OpenSSLInitializer& _open_ssl_initializer;
    std::unique_ptr< SSLContextMap > _ssl_ctx_map;
    // std::unordered_map< int, std::unique_ptr< SSL_CTX > > _id_to_ssl_ctx_map;

    void configure_individual(int id);
};

};  // namespace comm
