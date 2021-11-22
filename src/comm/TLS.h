/**
 * @copyright Copyright (c) 2021 ApertureData Inc.
 */

#pragma once

#include <string>
#include <tuple>

#include <openssl/ssl.h>

struct Certificate {
    std::string private_key;
    std::string cert;
};

class OpenSSLInitializer
{
public:
   static OpenSSLInitializer& instance();

private:
    OpenSSLInitializer();
};

SSL_CTX* create_client_context();
SSL_CTX* create_server_context();
Certificate generate_certificate();
void set_ca_certificate(SSL_CTX* ssl_ctx, const std::string& ca_certificate);
bool set_default_verify_paths(SSL_CTX* ssl_ctx);
void set_tls_certificate(SSL_CTX* ssl_ctx, const std::string& tls_certificate);
void set_tls_private_key(SSL_CTX* ssl_ctx, const std::string& tls_private_key);
