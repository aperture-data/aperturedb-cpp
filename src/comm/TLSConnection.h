/**
 * @copyright Copyright (c) 2021 ApertureData Inc.
 */

#pragma once

#include <memory>
#include <string>

#include "comm/Connection.h"
#include "util/Macros.h"
#include "comm/TLSSocket.h"

namespace comm
{

class TLSConnection : public Connection
{
   public:
    explicit TLSConnection(ConnMetrics* metrics = nullptr);
    explicit TLSConnection(std::unique_ptr< TLSSocket > tls_socket, ConnMetrics* metrics = nullptr);

    MOVEABLE_BY_DEFAULT(TLSConnection);
    NOT_COPYABLE(TLSConnection);

    std::string get_source() const override;
    short get_source_family() const override;
    std::string get_encryption() const override;

   protected:
    size_t read(uint8_t* buffer, size_t length) override;
    size_t write(const uint8_t* buffer, size_t length) override;

    std::unique_ptr< TLSSocket > _tls_socket;
};

};  // namespace comm
