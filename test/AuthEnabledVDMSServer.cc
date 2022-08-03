/**
 * @copyright Copyright (c) 2021 ApertureData Inc.
 */

#include "AuthEnabledVDMSServer.h"

#include <nlohmann/json.hpp>

#include "aperturedb/queryMessageWrapper.h"
#include "comm/Connection.h"
#include "comm/ConnServer.h"
#include "comm/Exception.h"

using namespace VDMS;

std::string AuthEnabledVDMSServer::random_string(size_t length)
{
    auto randchar = []() -> char
    {
        const char charset[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";

        const size_t max_index = sizeof(charset) - 1;

        return charset[rand() % max_index];
    };

    std::string str(length, 0);
    std::generate_n(str.begin(), length, randchar);

    return str;
}

AuthEnabledVDMSServer::AuthEnabledVDMSServer(int port, AuthEnabledVDMSServerConfig config) :
    _server(port, config.connServerConfig)
{
    auto thread_function = [&]()
    {
        std::shared_ptr<comm::Connection> server_conn = _server.accept();
        server_conn = _server.negotiate_protocol(server_conn);

        while (!_stop_signal) {
            protobufs::queryMessage protobuf_request;

            try {
                protobuf_request = receive_message(server_conn);
            } catch (comm::Exception exception) {
                if (exception.num == comm::ConnectionShutDown) {
                    return;
                }

                throw exception;
            }

            protobufs::queryMessage protobuf_response;

            auto is_authenticate_request_ = is_authenticate_request(protobuf_request);
            auto is_refresh_token_request_ = is_refresh_token_request(protobuf_request);

            if (is_authenticate_request_ || is_refresh_token_request_) {
                regenerate_tokens();

                auto command_name = is_authenticate_request_ ? "Authenticate" : "RefreshToken";

                auto responseJson = nlohmann::json::array({{
                    {command_name, {
                        {"session_token", session_token},
                        {"session_token_expires_in", config.session_token_expires_in},
                        {"refresh_token", refresh_token},
                        {"refresh_token_expires_in", config.refresh_token_expires_in},
                        {"status", 0}
                    }}
                }});

                protobuf_response.set_json(responseJson.dump());
            }
            else {
                if (protobuf_request.token() == session_token) {
                    protobuf_response.set_json(protobuf_request.json());
                }
                else {
                    auto responseJson = nlohmann::json::array({{
                        {"Something", {
                            {"status", -1}
                        }}
                    }});

                    protobuf_response.set_json(responseJson.dump());
                }
            }

            send_message(server_conn, protobuf_response);
        }
    };

    _work_thread = std::unique_ptr<std::thread>(new std::thread(thread_function));
}

AuthEnabledVDMSServer::~AuthEnabledVDMSServer()
{
    _stop_signal = true;

    if (_work_thread->joinable()) {
        _work_thread->join();
    }
}

bool AuthEnabledVDMSServer::is_authenticate_request(const protobufs::queryMessage& protobuf_request)
{
    auto requestJson = nlohmann::json::parse(protobuf_request.json());

    if (requestJson.is_array() && requestJson.size() == 1) {
        const auto& requestElement = requestJson.at(0);

        if (requestElement.is_object() && requestElement.contains("Authenticate")) {
            const auto& authenticateElement = requestElement["Authenticate"];

            if (authenticateElement.is_object() && authenticateElement.contains("username") && (authenticateElement.contains("password") || authenticateElement.contains("token"))) {
                return true;
            }
        }
    }

    return false;
}

bool AuthEnabledVDMSServer::is_refresh_token_request(const protobufs::queryMessage& protobuf_request)
{
    auto requestJson = nlohmann::json::parse(protobuf_request.json());

    if (requestJson.is_array() && requestJson.size() == 1) {
        const auto& requestElement = requestJson.at(0);

        if (requestElement.is_object() && requestElement.contains("RefreshToken")) {
            const auto& authenticateElement = requestElement["RefreshToken"];

            if (authenticateElement.is_object() && authenticateElement.contains("refresh_token")) {
                return true;
            }
        }
    }

    return false;
}

protobufs::queryMessage AuthEnabledVDMSServer::receive_message(const std::shared_ptr<comm::Connection>& connection)
{
    std::basic_string<uint8_t> message_received = connection->recv_message();

    protobufs::queryMessage protobuf_request;
    if (!protobuf_request.ParseFromArray(message_received.data(), message_received.length())) {
        THROW_EXCEPTION(ProtocolError, "Invalid message received");
    }

    return protobuf_request;
}

void AuthEnabledVDMSServer::regenerate_tokens()
{
    refresh_token = random_string(46);
    session_token = random_string(46);
}

void AuthEnabledVDMSServer::send_message(const std::shared_ptr<comm::Connection>& connection, const protobufs::queryMessage& protobuf_response)
{
    std::basic_string<uint8_t> message(protobuf_response.ByteSizeLong(), 0);
    protobuf_response.SerializeToArray(message.data(), message.length());

    connection->send_message(message.data(), message.length());
}
