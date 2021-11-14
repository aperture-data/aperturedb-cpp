/**
 * @copyright Copyright (c) 2021 ApertureData Inc.
 */

#include <chrono>
#include <string>
#include <thread>

#include "gtest/gtest.h"

#include "aperturedb/VDMSClient.h"
#include "AuthEnabledVDMSServer.h"
#include "comm/ConnServer.h"
#include "comm/ExceptionComm.h"
#include "comm/TLS.h"
#include "VDMSServer.h"

#define SERVER_PORT_INTERCHANGE 43444
#define NUMBER_OF_MESSAGES 20

class VDMSServerTests : public testing::Test {
protected:
    void SetUp() override
    {
        auto certificates = generate_certificate();

        std::string server_private_key = std::get<0>(certificates);
        std::string server_certificate = std::get<1>(certificates);

        connServerConfig = comm::ConnServerConfig{comm::Protocol::TLS, false, "", server_certificate, server_private_key};
    }

    comm::ConnServerConfig connServerConfig{};
};

// Ping-pong messages between server and client
TEST_F(VDMSServerTests, SyncMessages)
{
    std::string client_to_server("testing this awesome comm library with " \
                                 "come random data");

    VDMS::VDMSServer server(SERVER_PORT_INTERCHANGE, connServerConfig);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    VDMS::VDMSClient client("localhost", SERVER_PORT_INTERCHANGE, VDMS::Protocol::TLS, "");

    for (int i = 0; i < NUMBER_OF_MESSAGES; ++i) {
        // Send a query
        auto response = client.query(client_to_server.c_str());

        // Expect the same response
        ASSERT_EQ(0, response.json.compare(client_to_server));
    }
}

TEST_F(VDMSServerTests, SyncMessagesAuthenticated)
{
    std::string client_to_server = "[{}]";

    VDMS::AuthEnabledVDMSServerConfig config {connServerConfig};

    VDMS::AuthEnabledVDMSServer server(SERVER_PORT_INTERCHANGE, config);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    auto client = std::unique_ptr<VDMS::VDMSClient>(new VDMS::VDMSClient("username", "password", "localhost", SERVER_PORT_INTERCHANGE, VDMS::Protocol::TLS, ""));

    for (int i = 0; i < NUMBER_OF_MESSAGES; ++i) {
        // Send a query
        auto response = client->query(client_to_server.c_str());

        // Expect the same response
        ASSERT_EQ(0, response.json.compare(client_to_server));
    }
}

TEST_F(VDMSServerTests, SyncMessagesRefreshToken)
{
    std::string client_to_server = "[{}]";

    VDMS::AuthEnabledVDMSServerConfig config {connServerConfig, 60, 1};

    VDMS::AuthEnabledVDMSServer server(SERVER_PORT_INTERCHANGE, config);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    auto client = std::unique_ptr<VDMS::VDMSClient>(new VDMS::VDMSClient("username", "password", "localhost", SERVER_PORT_INTERCHANGE, VDMS::Protocol::TLS, ""));

    // Make sure the session token expires
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Send a query
    auto response = client->query(client_to_server.c_str());

    // Expect the same response
    ASSERT_EQ(0, response.json.compare(client_to_server));
}

TEST_F(VDMSServerTests, SyncMessagesReAuthenticate)
{
    std::string client_to_server = "[{}]";

    VDMS::AuthEnabledVDMSServerConfig config {connServerConfig, 2, 1};

    VDMS::AuthEnabledVDMSServer server(SERVER_PORT_INTERCHANGE, config);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    auto client = std::unique_ptr<VDMS::VDMSClient>(new VDMS::VDMSClient("username", "password", "localhost", SERVER_PORT_INTERCHANGE, VDMS::Protocol::TLS, ""));

    // Make sure the refresh token expires
    std::this_thread::sleep_for(std::chrono::seconds(3));

    // Send a query
    auto response = client->query(client_to_server.c_str());

    // Expect the same response
    ASSERT_EQ(0, response.json.compare(client_to_server));
}
