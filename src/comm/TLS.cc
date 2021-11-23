/**
 * @copyright Copyright (c) 2021 ApertureData Inc.
 */

#include <memory>

#include <openssl/err.h>

#include "comm/Exception.h"
#include "comm/TLS.h"

DEFINE_OPENSSL_DELETER(BIGNUM, BN_free);
DEFINE_OPENSSL_DELETER(BIO, BIO_free);
DEFINE_OPENSSL_DELETER(EVP_PKEY, EVP_PKEY_free);
DEFINE_OPENSSL_DELETER(RSA, RSA_free);
DEFINE_OPENSSL_DELETER(X509, X509_free);

std::string bio_to_string(const OpenSSLPointer<BIO>& bio);
OpenSSLPointer<BIO> string_to_bio(const std::string& string);

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

std::string bio_to_string(const OpenSSLPointer<BIO>& bio)
{
    std::string result;

    auto size = BIO_ctrl_pending(bio.get());
    result.resize(size);
    auto length = BIO_read(bio.get(), &result[0], size);
    result.resize(length);

    return result;
}

OpenSSLPointer<BIO> string_to_bio(const std::string& string)
{
    auto bio = BIO_new_mem_buf(static_cast<const void*>(string.c_str()), static_cast<int>(string.length()));

    return OpenSSLPointer<BIO>{ bio };
}

OpenSSLPointer<SSL_CTX> create_client_context()
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

    return OpenSSLPointer<SSL_CTX>{ ctx };
}

OpenSSLPointer<SSL_CTX> create_server_context()
{
    auto method = TLS_server_method();

    auto ctx = SSL_CTX_new(method);

    if (!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    return OpenSSLPointer<SSL_CTX>{ ctx };
}

Certificate generate_certificate()
{
    constexpr int rsa_key_length = 2048;
    constexpr int expires_in = 24 * 3600; // seconds

    OpenSSLPointer<RSA> rsa { RSA_new() };
    OpenSSLPointer<BIGNUM> bn { BN_new() };

    BN_set_word(bn.get(), RSA_F4);

    int res = RSA_generate_key_ex(rsa.get(), rsa_key_length, bn.get(), nullptr);

    if (res != 1) {
        THROW_EXCEPTION(TLSError, "Unable to create the private key");
    }

    OpenSSLPointer<X509> cert { X509_new() };
    OpenSSLPointer<EVP_PKEY> pkey { EVP_PKEY_new() };

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

    OpenSSLPointer<BIO> private_key_bio { BIO_new(BIO_s_mem()) };
    OpenSSLPointer<BIO> cert_bio { BIO_new(BIO_s_mem()) };

    res  = PEM_write_bio_PrivateKey(private_key_bio.get(), pkey.get(), nullptr, nullptr, 0, nullptr, nullptr);

    if (res != 1) {
        THROW_EXCEPTION(TLSError, "Unable to write the private key");
    }

    res = PEM_write_bio_X509(cert_bio.get(), cert.get());

    if (res != 1) {
        THROW_EXCEPTION(TLSError, "Unable to write the certificate");
    }

    Certificate c;
    c.private_key = bio_to_string(private_key_bio);
    c.cert        = bio_to_string(cert_bio);

    return c;
}

void set_ca_certificate(SSL_CTX* ssl_ctx, const std::string& ca_certificate)
{
    auto bio = string_to_bio(ca_certificate);

    if (!bio) {
        THROW_EXCEPTION(TLSError, "Unable to allocate memory for the certificate");
    }

    // TODO: add support for setting a passphrase
    OpenSSLPointer<X509> x509 { PEM_read_bio_X509(bio.get(), nullptr, nullptr, nullptr) };

    if (!x509) {
        THROW_EXCEPTION(TLSError, "Unable to read the certificate");
    }

    auto store = ::SSL_CTX_get_cert_store(ssl_ctx);

    auto res = ::X509_STORE_add_cert(store, x509.get());

    if (res != 1) {
        THROW_EXCEPTION(TLSError, "Unable to load the certificate");
    }
}

bool set_default_verify_paths(SSL_CTX* ssl_ctx)
{
    return ::SSL_CTX_set_default_verify_paths(ssl_ctx) == 1;
}

void set_tls_certificate(SSL_CTX* ssl_ctx, const std::string& tls_certificate)
{
    auto bio = string_to_bio(tls_certificate);

    if (!bio) {
        THROW_EXCEPTION(TLSError, "Unable to allocate memory for the certificate");
    }

    // TODO: add support for setting a passphrase
    OpenSSLPointer<X509> x509 { PEM_read_bio_X509_AUX(bio.get(), nullptr, nullptr, nullptr) };

    if (!x509) {
        THROW_EXCEPTION(TLSError, "Unable to read the certificate");
    }

    auto res = SSL_CTX_use_certificate(ssl_ctx, x509.get());

    if (res != 1) {
        THROW_EXCEPTION(TLSError, "Unable to load the certificate");
    }

    res = SSL_CTX_clear_chain_certs(ssl_ctx);

    if (res == 0) {
        THROW_EXCEPTION(TLSError, "Unable to load the certificate");
    }

    // TODO: add support for setting a passphrase
    OpenSSLPointer<X509> x509_ca { PEM_read_bio_X509(bio.get(), nullptr, nullptr, nullptr) };

    while (x509_ca) {
        res = SSL_CTX_add0_chain_cert(ssl_ctx, x509_ca.get());

        if (!res) {
            THROW_EXCEPTION(TLSError, "Unable to load the certificate");
        }

        // TODO: add support for setting a passphrase
        x509_ca.reset(PEM_read_bio_X509(bio.get(), nullptr, nullptr, nullptr));
    }
}

void set_tls_private_key(SSL_CTX* ssl_ctx, const std::string& tls_private_key)
{
    auto bio = string_to_bio(tls_private_key);

    if (!bio) {
        THROW_EXCEPTION(TLSError, "Unable to allocate memory for the certificate");
    }

    // TODO: add support for setting a passphrase
    auto rsa = PEM_read_bio_RSAPrivateKey(bio.get(), nullptr, nullptr, nullptr);

    if (!rsa) {
        THROW_EXCEPTION(TLSError, "Unable to read the certificate");
    }

    auto res = SSL_CTX_use_RSAPrivateKey(ssl_ctx, rsa);

    if (res != 1) {
        RSA_free(rsa);

        THROW_EXCEPTION(TLSError, "Unable to load the certificate");
    }
}