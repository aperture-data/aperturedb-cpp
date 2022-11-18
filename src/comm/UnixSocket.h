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
#include <sys/un.h>
#include <sys/socket.h>

#include "Socket.h"
#include "comm/Address.h"

namespace comm
{

class UnixSocket : public Socket
{
    friend class UnixConnection;

   public:
    UnixSocket(const UnixSocket&) = delete;
    ~UnixSocket();

    UnixSocket& operator=(const UnixSocket&) = delete;

    static std::unique_ptr< UnixSocket > create();
    std::unique_ptr< Socket > accept() override;

    bool bind(const std::string& path);
    bool connect(const std::string& path);
    bool listen() override;
    bool set_boolean_option(int level, int option_name, bool value) override;
    bool set_timeval_option(int level, int option_name, timeval value) override;
    void shutdown() override;

    std::string print_source() override;
    short source_family() override;
    int fd() const override { return _socket_fd; }

   private:
    explicit UnixSocket(int socket_fd, sockaddr_un);

    int _socket_fd{-1};
    short _source_family{AF_UNSPEC};
    struct sockaddr_un _source;
};

};  // namespace comm
