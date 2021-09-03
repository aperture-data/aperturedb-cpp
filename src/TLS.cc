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

#include <memory>

#include <openssl/err.h>

#include "ExceptionComm.h"
#include "TLS.h"

std::string bio_to_string(const std::unique_ptr<BIO, decltype(&BIO_free)>& bio);

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

std::string bio_to_string(const std::unique_ptr<BIO, decltype(&BIO_free)>& bio)
{
    std::string result;

    auto size = BIO_ctrl_pending(bio.get());
    result.resize(size);
    auto length = BIO_read(bio.get(), &result[0], size);
    result.resize(length);

    return result;
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

std::tuple<std::string, std::string> generate_certificate()
{
    constexpr int rsa_key_length = 2048;
    constexpr int expires_in = 24 * 3600; // seconds

    std::unique_ptr<RSA, void(*)(RSA*)> rsa { RSA_new(), RSA_free };
    std::unique_ptr<BIGNUM, void(*)(BIGNUM*)> bn { BN_new(), BN_free };

    BN_set_word(bn.get(), RSA_F4);

    int res = RSA_generate_key_ex(rsa.get(), rsa_key_length, bn.get(), nullptr);

    if (res != 1) {
        throw ExceptionComm(TLSError, "Unable to create the private key");
    }

    std::unique_ptr<X509, void(*)(X509*)> cert { X509_new(), X509_free };
    std::unique_ptr<EVP_PKEY, void(*)(EVP_PKEY*)> pkey { EVP_PKEY_new(), EVP_PKEY_free};

    EVP_PKEY_assign(pkey.get(), EVP_PKEY_RSA, reinterpret_cast<char*>(rsa.release()));
    ASN1_INTEGER_set(X509_get_serialNumber(cert.get()), 1);

    X509_gmtime_adj(X509_get_notBefore(cert.get()), 0); // now
    X509_gmtime_adj(X509_get_notAfter(cert.get()), expires_in);

    X509_set_pubkey(cert.get(), pkey.get());

    auto name = X509_get_subject_name(cert.get());

    constexpr unsigned char country[] = "US";
    constexpr unsigned char company[] = "ApertureData Inc.";
    constexpr unsigned char common_name[] = "localhost";

    X509_NAME_add_entry_by_txt(name, "C",  MBSTRING_ASC, country, -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "O",  MBSTRING_ASC, company, -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, common_name, -1, -1, 0);

    X509_set_issuer_name(cert.get(), name);
    X509_sign(cert.get(), pkey.get(), EVP_sha256());

    std::unique_ptr<BIO, decltype(&BIO_free)> private_key_bio { BIO_new(BIO_s_mem()), BIO_free };
    std::unique_ptr<BIO, int(*)(BIO*)> cert_bio { BIO_new(BIO_s_mem()), BIO_free };

    res  = PEM_write_bio_PrivateKey(private_key_bio.get(), pkey.get(), nullptr, nullptr, 0, nullptr, nullptr);

    if (res != 1) {
        throw ExceptionComm(TLSError, "Unable to write the private key");
    }

    res = PEM_write_bio_X509(cert_bio.get(), cert.get());

    if (res != 1) {
        throw ExceptionComm(TLSError, "Unable to write the certificate");
    }

    return {bio_to_string(private_key_bio), bio_to_string(cert_bio)};
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

bool set_default_verify_paths(SSL_CTX* ssl_ctx)
{
    return ::SSL_CTX_set_default_verify_paths(ssl_ctx) == 1;
}

void set_tls_certificate(SSL_CTX* ssl_ctx, const std::string& tls_certificate)
{
    auto bio = BIO_new_mem_buf(static_cast<const void*>(tls_certificate.c_str()), static_cast<int>(tls_certificate.length()));

    if (!bio)
    {
        throw ExceptionComm(TLSError, "Unable to allocate memory for the certificate");
    }

    // TODO: add support for setting a passphrase
    auto x509 = PEM_read_bio_X509_AUX(bio, nullptr, nullptr, nullptr);

    if (!x509)
    {
        BIO_free(bio);

        throw ExceptionComm(TLSError, "Unable to read the certificate");
    }

    auto cleanup = [&]() {
        X509_free(x509);
        BIO_free(bio);
    };

    auto res = SSL_CTX_use_certificate(ssl_ctx, x509);

    if (res != 1)
    {
        cleanup();

        throw ExceptionComm(TLSError, "Unable to load the certificate");
    }

    res = SSL_CTX_clear_chain_certs(ssl_ctx);

    if (res == 0) {
        cleanup();

        throw ExceptionComm(TLSError, "Unable to load the certificate");
    }

    X509* ca{nullptr};

    // TODO: add support for setting a passphrase
    while ((ca = PEM_read_bio_X509(bio, nullptr, nullptr, nullptr)) != NULL) {
        res = SSL_CTX_add0_chain_cert(ssl_ctx, ca);
        
        if (!res) {
            X509_free(ca);
            
            cleanup();

            throw ExceptionComm(TLSError, "Unable to load the certificate");
        }
    }

    if (ca) {
        X509_free(ca);
    }

    cleanup();
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