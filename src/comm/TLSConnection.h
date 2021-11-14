/**
 * @copyright Copyright (c) 2021 ApertureData Inc.
 */

#pragma once

#include <memory>
#include <string>

#include "comm/Connection.h"
#include "comm/TLSSocket.h"

namespace comm {

    class TLSConnection : public Connection
    {
    public:

        TLSConnection();
        explicit TLSConnection(std::unique_ptr<TLSSocket> tls_socket);
        TLSConnection(TLSConnection&&) = default;
        TLSConnection(const TLSConnection&) = delete;

        TLSConnection& operator=(TLSConnection&&) = default;
        TLSConnection& operator=(const TLSConnection&) = delete;

    protected:

        size_t read(uint8_t* buffer, size_t length) override;
        size_t write(const uint8_t* buffer, size_t length) override;

        std::unique_ptr<TLSSocket> _tls_socket;
    };

};
