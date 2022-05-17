/**
 * @copyright Copyright (c) 2021 ApertureData Inc.
 */

#include <string>
#include <thread>

#include "gtest/gtest.h"

#include "Barrier.h"
#include "comm/ConnClient.h"
#include "comm/ConnServer.h"
#include "comm/Exception.h"
#include "comm/TLS.h"

#define SERVER_PORT_INTERCHANGE 43444
#define SERVER_PORT_MULTIPLE    43444
#define NUMBER_OF_MESSAGES 20

typedef std::basic_string<uint8_t> BytesBuffer;

class TLSConnectionTests : public testing::Test {
protected:
    void SetUp() override
    {
        auto certificates = generate_certificate();

        std::string server_private_key = certificates.private_key;
        std::string server_certificate = certificates.cert;

        connClientConfig = comm::ConnClientConfig{comm::Protocol::TLS, "", false};
        connServerConfig = comm::ConnServerConfig{comm::Protocol::TLS, false, "", server_certificate, server_private_key};
    }

    comm::ConnClientConfig connClientConfig{};
    comm::ConnServerConfig connServerConfig{};
};

// Ping-pong messages between server and client
TEST_F(TLSConnectionTests, SyncMessages)
{
    std::string client_to_server("testing this awesome comm library with some random data");
    std::string server_to_client("this awesome library seems to work :)");

    Barrier barrier(2);

    std::thread server_thread([&]()
    {
        comm::ConnServer server(SERVER_PORT_INTERCHANGE, connServerConfig);

        barrier.wait();

        auto server_conn = server.accept();

        for (int i = 0; i < NUMBER_OF_MESSAGES; ++i) {
            //Recieve something
            BytesBuffer message_received = server_conn->recv_message();
            std::string recv_message(reinterpret_cast<char*>(message_received.data()));
            ASSERT_EQ(0, recv_message.compare(client_to_server));

            //Send something
            server_conn->send_message(reinterpret_cast<const uint8_t*>(server_to_client.c_str()),
                                     server_to_client.length());
        }
    });

    comm::ConnClient conn_client({"localhost", SERVER_PORT_INTERCHANGE}, connClientConfig);

    barrier.wait();

    auto connection = conn_client.connect();

    for (int i = 0; i < NUMBER_OF_MESSAGES; ++i) {
        // Send something
        connection->send_message(reinterpret_cast<const uint8_t*>(client_to_server.c_str()),
                                 client_to_server.length());

        // Receive something
        BytesBuffer message_received = connection->recv_message();
        std::string recv_message(reinterpret_cast<char*>(message_received.data()));
        ASSERT_EQ(0, recv_message.compare(server_to_client));
    }

    server_thread.join();
}

// Both client and server send all messages firsts and then check the received messages.
TEST_F(TLSConnectionTests, AsyncMessages)
{
    std::string client_to_server("client sends some random data");
    std::string server_to_client("this library seems to work :)");

    Barrier barrier(2);

    std::thread server_thread([&]()
    {
        comm::ConnServer server(SERVER_PORT_MULTIPLE, connServerConfig);

        barrier.wait();

        auto server_conn = server.accept();

        for (int i = 0; i < NUMBER_OF_MESSAGES; ++i) {
            //Send something
            server_conn->send_message(reinterpret_cast<const uint8_t*>(server_to_client.c_str()),
                                     server_to_client.length());
        }

        for (int i = 0; i < NUMBER_OF_MESSAGES; ++i) {
            //Recieve something
            BytesBuffer message_received = server_conn->recv_message();
            std::string recv_message(reinterpret_cast<char*>(message_received.data()));
            ASSERT_EQ(0, recv_message.compare(client_to_server));
        }
    });

    comm::ConnClient conn_client({"localhost", SERVER_PORT_MULTIPLE}, connClientConfig);

    barrier.wait();

    auto connection = conn_client.connect();

    for (int i = 0; i < NUMBER_OF_MESSAGES; ++i) {
        // Send something
        connection->send_message(reinterpret_cast<const uint8_t*>(client_to_server.c_str()),
                                 client_to_server.length());
    }

    for (int i = 0; i < NUMBER_OF_MESSAGES; ++i) {
        // Receive something
        BytesBuffer message_received = connection->recv_message();
        std::string recv_message(reinterpret_cast<char*>(message_received.data()));
        ASSERT_EQ(0, recv_message.compare(server_to_client));
    }

    server_thread.join();
}

// Server accepts connection and then goes down, client tries to recv.
TEST_F(TLSConnectionTests, ServerShutdownRecv)
{
    Barrier barrier(2);

    std::thread server_thread([&]()
    {
        comm::ConnServer server(SERVER_PORT_INTERCHANGE, connServerConfig);

        barrier.wait();

        server.accept();
    });

    comm::ConnClient conn_client({"localhost", SERVER_PORT_INTERCHANGE}, connClientConfig);

    barrier.wait();

    auto connection = conn_client.connect();

    server_thread.join(); // Here the server will close the port.

    ASSERT_THROW(
        connection->recv_message(),
        comm::Exception
    );
}

TEST_F(TLSConnectionTests, SendArrayInts)
{
    int arr[10] = {22, 568, 254, 784, 452, 458, 235, 124, 1425, 1542};

    Barrier barrier(2);

    std::thread server_thread([&]()
    {
        comm::ConnServer server(SERVER_PORT_INTERCHANGE, connServerConfig);

        barrier.wait();

        auto server_conn = server.accept();

        server_conn->send_message(reinterpret_cast<const uint8_t*>(arr), sizeof(arr));
    });

    comm::ConnClient conn_client({"localhost", SERVER_PORT_INTERCHANGE}, connClientConfig);

    barrier.wait();

    auto connection = conn_client.connect();

    BytesBuffer message_received = connection->recv_message();

    const int *arr_recv = reinterpret_cast<const int*>(message_received.data());
    for (int i = 0; i < 10; ++i) {
        ASSERT_EQ(arr[i], arr_recv[i]);
    }

    server_thread.join();
}

TEST_F(TLSConnectionTests, Unreachable)
{
    comm::ConnClient client_1({"unreachable.com.ar.something", 5555});

    ASSERT_THROW (
        client_1.connect(),
        comm::Exception
    );

    comm::ConnClient client_2({"localhost", -1});

    ASSERT_THROW (
        client_2.connect(),
        comm::Exception
    );
}

TEST_F(TLSConnectionTests, ServerWrongPort)
{
    ASSERT_THROW (
        comm::ConnServer server(-22, connServerConfig),
        comm::Exception
    );

    ASSERT_THROW (
        comm::ConnServer server(0, connServerConfig),
        comm::Exception
    );
}

TEST_F(TLSConnectionTests, ClientWrongAddrOrPort)
{
    comm::ConnClient client_1({"", 3424});

    ASSERT_THROW (
        client_1.connect(),
        comm::Exception
    );

    comm::ConnClient client_2({"intel.com", -32});

    ASSERT_THROW (
        client_2.connect(),
        comm::Exception
    );

    comm::ConnClient client_3({"intel.com", 0});

    ASSERT_THROW (
        client_3.connect(),
        comm::Exception
    );
}

TEST_F(TLSConnectionTests, UseAutogeneratedCertificates)
{
    Barrier barrier(2);

    std::thread server_thread([&]()
    {
        static const comm::ConnServerConfig connServerConfig_{comm::Protocol::TLS, true};

        comm::ConnServer server(SERVER_PORT_INTERCHANGE, connServerConfig_);

        barrier.wait();

        ASSERT_NO_THROW(server.accept());
    });

    static const comm::ConnClientConfig connClientConfig_{comm::Protocol::TLS, "", false};

    comm::ConnClient conn_client({"localhost", SERVER_PORT_INTERCHANGE}, connClientConfig_);

    barrier.wait();

    ASSERT_NO_THROW(conn_client.connect());

    server_thread.join();
}
