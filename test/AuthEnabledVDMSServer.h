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

#include <atomic>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "ConnServer.h"

namespace VDMS {

    namespace protobufs {

        class queryMessage;

    }

    struct AuthEnabledVDMSServerConfig {
        comm::ConnServerConfig connServerConfig{};
        int32_t refresh_token_expires_in = 24 * 60 * 60;
        int32_t session_token_expires_in = 60 * 60;

        AuthEnabledVDMSServerConfig(comm::ConnServerConfig connServerConfig_ = {}, int32_t refresh_token_expires_in_ = 24 * 60 * 60, int32_t session_token_expires_in_ = 60 * 60) :
            connServerConfig(connServerConfig_),
            refresh_token_expires_in(refresh_token_expires_in_),
            session_token_expires_in(session_token_expires_in_)
        {
        }
    };

    class AuthEnabledVDMSServer {
        std::unique_ptr<comm::ConnServer> _server;

    public:
        AuthEnabledVDMSServer(int port, AuthEnabledVDMSServerConfig config);
        ~AuthEnabledVDMSServer();

    private:

        bool is_authenticate_request(const protobufs::queryMessage& protobuf_request);
        bool is_refresh_token_request(const protobufs::queryMessage& protobuf_request);
        protobufs::queryMessage receive_message(const std::shared_ptr<comm::Connection>& connection);
        void regenerate_tokens();
        void send_message(const std::shared_ptr<comm::Connection>& connection, const protobufs::queryMessage& protobuf_response);

        std::atomic<bool> _stop_signal{false};
        std::unique_ptr<std::thread> _work_thread{};
        std::string session_token{};
        std::string refresh_token{};
    };
};
