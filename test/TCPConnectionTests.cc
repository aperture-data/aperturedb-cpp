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

#include <string>
#include <thread>

#include "gtest/gtest.h"

#include "ConnClient.h"
#include "ConnServer.h"
#include "ExceptionComm.h"
#include "TCPConnection.h"

#define SERVER_PORT_INTERCHANGE 43444
#define SERVER_PORT_MULTIPLE    43444
#define NUMBER_OF_MESSAGES 20

typedef std::basic_string<uint8_t> BytesBuffer;

// Ping-pong messages between server and client
TEST(TCPConnectionTests, SyncMessages)
{
    std::string client_to_server("testing this awesome comm library with " \
                                 "come random data");
    std::string server_to_client("this awesome library seems to work :)");

    std::thread server_thread([client_to_server, server_to_client]()
    {
        comm::ConnServer server(SERVER_PORT_INTERCHANGE);
        auto server_conn = server.accept();

        for (int i = 0; i < NUMBER_OF_MESSAGES; ++i) {
            //Recieve something
            BytesBuffer message_received = server_conn->recv_message();
            std::string recv_message(reinterpret_cast<char*>(const_cast<uint8_t*>(message_received.data())));
            ASSERT_EQ(0, recv_message.compare(client_to_server));

            //Send something
            server_conn->send_message(reinterpret_cast<const uint8_t*>(server_to_client.c_str()),
                                     server_to_client.length());
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    comm::ConnClient conn_client({"localhost", SERVER_PORT_INTERCHANGE});

    auto connection = conn_client.connect();

    for (int i = 0; i < NUMBER_OF_MESSAGES; ++i) {
        // Send something
        connection->send_message(reinterpret_cast<const uint8_t*>(client_to_server.c_str()),
                                 client_to_server.length());

        // Receive something
        BytesBuffer message_received = connection->recv_message();
        std::string recv_message(reinterpret_cast<char*>(const_cast<uint8_t*>(message_received.data())));
        ASSERT_EQ(0, recv_message.compare(server_to_client));
    }

    server_thread.join();
}

// Both client and server send all messages firsts and then check the received messages.
TEST(TCPConnectionTests, AsyncMessages)
{
    std::string client_to_server("client sends some random data");
    std::string server_to_client("this library seems to work :)");

    std::thread server_thread([client_to_server, server_to_client]()
    {
        comm::ConnServer server(SERVER_PORT_MULTIPLE);
        auto server_conn = server.accept();

        for (int i = 0; i < NUMBER_OF_MESSAGES; ++i) {
            //Send something
            server_conn->send_message(reinterpret_cast<const uint8_t*>(server_to_client.c_str()),
                                     server_to_client.length());
        }

        for (int i = 0; i < NUMBER_OF_MESSAGES; ++i) {
            //Recieve something
            BytesBuffer message_received = server_conn->recv_message();
            std::string recv_message(reinterpret_cast<char*>(const_cast<uint8_t*>(message_received.data())));
            ASSERT_EQ(0, recv_message.compare(client_to_server));
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    comm::ConnClient conn_client({"localhost", SERVER_PORT_MULTIPLE});

    auto connection = conn_client.connect();

    for (int i = 0; i < NUMBER_OF_MESSAGES; ++i) {
        // Send something
        connection->send_message(reinterpret_cast<const uint8_t*>(client_to_server.c_str()),
                                 client_to_server.length());
    }

    for (int i = 0; i < NUMBER_OF_MESSAGES; ++i) {
        // Receive something
        BytesBuffer message_received = connection->recv_message();
        std::string recv_message(reinterpret_cast<char*>(const_cast<uint8_t*>(message_received.data())));
        ASSERT_EQ(0, recv_message.compare(server_to_client));
    }

    server_thread.join();
}

// Server accepts connection and then goes down, client tries to recv.
TEST(TCPConnectionTests, ServerShutdownRecv)
{
    std::thread server_thread([]()
    {
        comm::ConnServer server(SERVER_PORT_INTERCHANGE);
        server.accept();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    comm::ConnClient conn_client({"localhost", SERVER_PORT_INTERCHANGE});

    auto connection = conn_client.connect();

    server_thread.join(); // Here the server will close the port.

    ASSERT_THROW(
        connection->recv_message(),
        comm::Exception
    );
}

TEST(TCPConnectionTests, SendArrayInts)
{
    int arr[10] = {22, 568, 254, 784, 452, 458, 235, 124, 1425, 1542};
    std::thread server_thread([arr]()
    {
        comm::ConnServer server(SERVER_PORT_INTERCHANGE);
        auto server_conn = server.accept();

        server_conn->send_message(reinterpret_cast<const uint8_t*>(arr), sizeof(arr));
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    comm::ConnClient conn_client({"localhost", SERVER_PORT_INTERCHANGE});

    auto connection = conn_client.connect();

    BytesBuffer message_received = connection->recv_message();

    const int* arr_recv = reinterpret_cast<const int*>(message_received.data());
    for (int i = 0; i < 10; ++i) {
        ASSERT_EQ(arr[i], arr_recv[i]);
    }

    server_thread.join();
}

TEST(TCPConnectionTests, MoveCopy)
{
    comm::TCPConnection a;
    comm::TCPConnection server_conn;
    server_conn = std::move(a); // Testing copy with move works
}

TEST(TCPConnectionTests, Unreachable)
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

TEST(TCPConnectionTests, ServerWrongPort)
{
    ASSERT_THROW (
        comm::ConnServer server(-22),
        comm::Exception
    );

    ASSERT_THROW (
        comm::ConnServer server(0),
        comm::Exception
    );
}

TEST(TCPConnectionTests, ClientWrongAddrOrPort)
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

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    // To make GoogleTest silent:
    // if (true) {
    //     auto& listeners = ::testing::UnitTest::GetInstance()->listeners();
    //     delete listeners.Release(listeners.default_result_printer());
    // }
    return RUN_ALL_TESTS();
}
