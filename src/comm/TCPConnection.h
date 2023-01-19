/**
 * @section LICENSE
 *
 * The MIT License
 *
 * @copyright Copyright (c) 2017 Intel Corporation
 * @copyright Copyright (c) 2021 ApertureData Inc
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

#pragma once

#include <memory>
#include <string>

#include "comm/Connection.h"
#include "util/Macros.h"
#include "comm/TCPSocket.h"

namespace comm
{

class TCPConnection : public Connection
{
    friend class TLSConnClient;

   public:
    explicit TCPConnection(std::unique_ptr< TCPSocket > tcp_socket, ConnMetrics* metrics = nullptr);

    MOVEABLE_BY_DEFAULT(TCPConnection);
    NOT_COPYABLE(TCPConnection);

    std::unique_ptr< TCPSocket > release_socket();
    void shutdown();
    std::string get_source() const override;
    short get_source_family() const override;
    std::string get_encryption() const override;
    bool is_open() override;

   protected:
    size_t read(uint8_t* buffer, size_t length) override;
    size_t write(const uint8_t* buffer, size_t length) override;

    std::unique_ptr< TCPSocket > _tcp_socket;
};

};  // namespace comm
