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
#include <netinet/ip.h>

#include "comm/Address.h"

namespace comm
{

class TCPSocket
{
    friend class TCPConnection;
    friend class TLSSocket;

   public:
    TCPSocket(const TCPSocket&) = delete;
    ~TCPSocket();

    TCPSocket& operator=(const TCPSocket&) = delete;

    static std::unique_ptr< TCPSocket > create();
    static std::unique_ptr< TCPSocket > accept(
        const std::unique_ptr< TCPSocket >& listening_socket);

    bool bind(int port);
    bool connect(const Address& address);
    bool listen();
    bool set_boolean_option(int level, int option_name, bool value);
    bool set_timeval_option(int level, int option_name, timeval value);
    void shutdown();

    std::string print_source();
    short source_family();

   private:
    explicit TCPSocket(int socket_fd, sockaddr_in);

    int _socket_fd{-1};
    short _source_family{AF_UNSPEC};
    struct sockaddr_in _source;
};

};  // namespace comm
