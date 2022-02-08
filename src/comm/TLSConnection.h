/**
 * @copyright Copyright (c) 2021 ApertureData Inc.
 */

#pragma once

#include <memory>
#include <string>

#include "comm/Connection.h"
#include "comm/Macros.h"
#include "comm/TLSSocket.h"

namespace comm {

    class TLSConnection : public Connection
    {
    public:

        explicit TLSConnection(ConnMetrics* metrics = nullptr);
        explicit TLSConnection(std::unique_ptr<TLSSocket> tls_socket,
            ConnMetrics* metrics = nullptr);

        MOVEABLE_BY_DEFAULT(TLSConnection);
        NOT_COPYABLE(TLSConnection);

    protected:

        size_t read(uint8_t* buffer, size_t length) override;
        size_t write(const uint8_t* buffer, size_t length) override;

        std::unique_ptr<TLSSocket> _tls_socket;
    };

};
