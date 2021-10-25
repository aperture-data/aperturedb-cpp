/**
 * @section LICENSE
 *
 * The MIT License
 *
 * @copyright Copyright (c) 2017 Intel Corporation
 * @copyright Copyright (c) 2021 ApertureData
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

#include "AuthEnabledVDMSServer.h"

#include <nlohmann/json.hpp>

#include "gcc_util.h" // DISABLE_WARNING
DISABLE_WARNING(effc++)
DISABLE_WARNING(useless-cast)
DISABLE_WARNING(suggest-override)
#include "queryMessage.pb.h"
ENABLE_WARNING(suggest-override)
ENABLE_WARNING(useless-cast)
ENABLE_WARNING(effc++)

#include "Connection.h"
#include "ConnServer.h"
#include "ExceptionComm.h"

using namespace VDMS;

std::string random_string(size_t length);

std::string random_string(size_t length)
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
    _server(new comm::ConnServer(port, config.connServerConfig))
{
    auto thread_function = [&]()
    {
        std::shared_ptr<comm::Connection> server_conn = _server->accept();

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
        auto requestElement = requestJson.at(0);

        if (requestElement.is_object() && requestElement.contains("Authenticate")) {
            auto authenticateElement = requestElement["Authenticate"];

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
        auto requestElement = requestJson.at(0);

        if (requestElement.is_object() && requestElement.contains("RefreshToken")) {
            auto authenticateElement = requestElement["RefreshToken"];

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
    protobuf_request.ParseFromArray(message_received.data(), message_received.length());

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
    protobuf_response.SerializeToArray(const_cast<uint8_t*>(message.data()), message.length());

    connection->send_message(message.data(), message.length());
}
