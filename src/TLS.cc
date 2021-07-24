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

#include <openssl/err.h>

#include "ExceptionComm.h"
#include "TLS.h"

OpenSSLInitializer::OpenSSLInitializer()
{
    OPENSSL_init_ssl(OPENSSL_INIT_LOAD_SSL_STRINGS | OPENSSL_INIT_LOAD_CRYPTO_STRINGS, nullptr);

    // Disable compression to avoid CRIME attacks
    sk_SSL_COMP_zero(SSL_COMP_get_compression_methods());
}

OpenSSLInitializer& OpenSSLInitializer::instance()
{
    static OpenSSLInitializer initializer{};

    return initializer;
}

SSL_CTX* create_client_context()
{
    const SSL_METHOD* method;
    SSL_CTX* ctx;

    method = TLS_client_method();

    ctx = SSL_CTX_new(method);

    if (!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    return ctx;
}

SSL_CTX* create_server_context()
{
    auto method = TLS_server_method();

    auto ctx = SSL_CTX_new(method);

    if (!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    return ctx;
}

void set_ca_certificate(SSL_CTX* ssl_ctx, const std::string& ca_certificate)
{
    auto bio = BIO_new_mem_buf(static_cast<const void*>(ca_certificate.c_str()), static_cast<int>(ca_certificate.length()));

    if (!bio)
    {
        throw ExceptionComm(TLSError, "Unable to allocate memory for the certificate");
    }

    // TODO: add support for setting a passphrase
    auto x509 = PEM_read_bio_X509(bio, nullptr, nullptr, nullptr);

    if (!x509)
    {
        BIO_free(bio);

        throw ExceptionComm(TLSError, "Unable to read the certificate");
    }

    auto store = ::SSL_CTX_get_cert_store(ssl_ctx);

    auto res = ::X509_STORE_add_cert(store, x509);

    if (res != 1)
    {
        X509_free(x509);
        BIO_free(bio);

        throw ExceptionComm(TLSError, "Unable to load the certificate");
    }
}

void set_tls_certificate(SSL_CTX* ssl_ctx, const std::string& tls_certificate)
{
    auto bio = BIO_new_mem_buf(static_cast<const void*>(tls_certificate.c_str()), static_cast<int>(tls_certificate.length()));

    if (!bio)
    {
        throw ExceptionComm(TLSError, "Unable to allocate memory for the certificate");
    }

    // TODO: add support for setting a passphrase
    auto x509 = PEM_read_bio_X509(bio, nullptr, nullptr, nullptr);

    if (!x509)
    {
        BIO_free(bio);

        throw ExceptionComm(TLSError, "Unable to read the certificate");
    }

    auto res = SSL_CTX_use_certificate(ssl_ctx, x509);

    if (res != 1)
    {
        X509_free(x509);
        BIO_free(bio);

        throw ExceptionComm(TLSError, "Unable to load the certificate");
    }
}

void set_tls_private_key(SSL_CTX* ssl_ctx, const std::string& tls_private_key)
{
    auto bio = BIO_new_mem_buf(static_cast<const void*>(tls_private_key.c_str()), static_cast<int>(tls_private_key.length()));

    if (!bio)
    {
        throw ExceptionComm(TLSError, "Unable to allocate memory for the certificate");
    }

    // TODO: add support for setting a passphrase
    auto rsa = PEM_read_bio_RSAPrivateKey(bio, nullptr, nullptr, nullptr);

    if (!rsa)
    {
        BIO_free(bio);

        throw ExceptionComm(TLSError, "Unable to read the certificate");
    }

    auto res = SSL_CTX_use_RSAPrivateKey(ssl_ctx, rsa);

    if (res != 1)
    {
        RSA_free(rsa);
        BIO_free(bio);

        throw ExceptionComm(TLSError, "Unable to load the certificate");
    }
}