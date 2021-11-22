/**
 * @copyright Copyright (c) 2021 ApertureData Inc.
 */

#pragma once

#include <atomic>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "comm/ConnServer.h"

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

        std::string random_string(size_t length);

        std::atomic<bool> _stop_signal{false};
        std::unique_ptr<std::thread> _work_thread{};
        std::string session_token{};
        std::string refresh_token{};
    };
};
