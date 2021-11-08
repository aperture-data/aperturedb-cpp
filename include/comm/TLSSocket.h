/**
 * @copyright Copyright (c) 2021 ApertureData Inc.
 */

#pragma once

#include <memory>
#include <string>

#include <openssl/ssl.h>

#include "TCPSocket.h"

namespace comm {

    class TLSSocket
    {
        friend class TLSConnection;

    public:

        TLSSocket(const TLSSocket&) = delete;
        ~TLSSocket();

        TLSSocket& operator=(const TLSSocket&) = delete;

        static std::unique_ptr<TLSSocket> create(std::unique_ptr<TCPSocket> tcp_socket, SSL_CTX* ssl_ctx);

        void accept();
        void connect();

    private:

        explicit TLSSocket(std::unique_ptr<TCPSocket> tcp_socket, SSL* ssl);

        SSL* _ssl{nullptr};
        std::unique_ptr<TCPSocket> _tcp_socket;
    };

};
