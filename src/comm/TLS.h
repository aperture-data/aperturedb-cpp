/**
 * @copyright Copyright (c) 2021 ApertureData Inc.
 */

#pragma once

#include <memory>
#include <string>
#include <tuple>

#include <openssl/ssl.h>

struct Certificate {
    std::string private_key{};
    std::string cert{};
};

class OpenSSLInitializer
{
   public:
    static OpenSSLInitializer& instance();

   private:
    OpenSSLInitializer();
};

template < typename T >
struct OpenSSLDeleter {
};

#define DEFINE_OPENSSL_DELETER(Type, FreeFunc)                 \
    template <>                                                \
    struct OpenSSLDeleter< Type > {                            \
        void operator()(Type* obj) noexcept { FreeFunc(obj); } \
    };

DEFINE_OPENSSL_DELETER(SSL_CTX, SSL_CTX_free);

template < typename T >
using OpenSSLPointer = std::unique_ptr< T, OpenSSLDeleter< T > >;

OpenSSLPointer< SSL_CTX > create_client_context();
OpenSSLPointer< SSL_CTX > create_server_context();
Certificate generate_certificate();
void set_ca_certificate(SSL_CTX* ssl_ctx, const std::string& ca_certificate);
bool set_default_verify_paths(SSL_CTX* ssl_ctx);
void set_tls_certificate(SSL_CTX* ssl_ctx, const std::string& tls_certificate);
void set_tls_private_key(SSL_CTX* ssl_ctx, const std::string& tls_private_key);
