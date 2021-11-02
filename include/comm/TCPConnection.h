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

#pragma once

#include <memory>
#include <string>

#include "Connection.h"
#include "TCPSocket.h"

namespace comm {

    class TCPConnection : public Connection
    {

        friend class TLSConnClient;

    public:

        TCPConnection();
        explicit TCPConnection(std::unique_ptr<TCPSocket> tcp_socket);
        TCPConnection(TCPConnection&&) = default;
        TCPConnection(const TCPConnection&) = delete;

        TCPConnection& operator=(TCPConnection&&) = default;
        TCPConnection& operator=(const TCPConnection&) = delete;

        std::unique_ptr<TCPSocket> release_socket();
        void shutdown();

    protected:

        size_t read(uint8_t* buffer, size_t length) override;
        size_t write(const uint8_t* buffer, size_t length) override;

        std::unique_ptr<TCPSocket> _tcp_socket;
    };

};
