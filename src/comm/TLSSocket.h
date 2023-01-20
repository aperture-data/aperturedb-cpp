/**
 * @copyright Copyright (c) 2021 ApertureData Inc.
 */

#pragma once

#include <memory>
#include <string>

#include <openssl/ssl.h>

#include "TCPSocket.h"

namespace comm
{

class TLSSocket
{
    friend class TLSConnection;

   public:
    TLSSocket(const TLSSocket&) = delete;
    ~TLSSocket();

    TLSSocket& operator=(const TLSSocket&) = delete;

    static std::unique_ptr< TLSSocket > create(std::unique_ptr< TCPSocket > tcp_socket,
                                               const std::shared_ptr< SSL_CTX >& ssl_ctx);

    void accept();
    void connect();

    std::string print_source();
    short source_family();

   private:
    explicit TLSSocket(std::unique_ptr< TCPSocket > tcp_socket, SSL* ssl);

    SSL* _ssl{nullptr};

    // Even if this member is not used, it is necessary to keep it alive
    // until the destructor is called.
    std::unique_ptr< TCPSocket > _tcp_socket;
};

};  // namespace comm
