/**
 * @copyright Copyright (c) 2021 ApertureData Inc.
 */

#include <string>
#include <thread>
#include <stdlib.h>

#include "gtest/gtest.h"

#include "Barrier.h"
#include "comm/ConnClient.h"
#include "comm/ConnServer.h"
#include "comm/Exception.h"
#include "comm/UnixConnection.h"

#define NUMBER_OF_MESSAGES 20
#define NUMBER_OF_CLIENTS  4

typedef std::basic_string< uint8_t > BytesBuffer;

#define UNIX_SOCKET_PATH_TEMPLATE "/tmp/aperturedb-unix-socket.XXXXXX"
class UnixSocketLocker
{
   public:
    UnixSocketLocker(std::string path_template)
    {
        path = path_template;
        // mkstemp modifies input, returns path that way.
        int tmp_fd = mkstemp(path.data());

        if (tmp_fd == -1) {
            throw std::runtime_error("Failed to Create Unix Socket Path");
        }
	// must unlink so a bind works.
        unlink(path.c_str());
        fd = tmp_fd;
    }
    // close fd when locker exits
    ~UnixSocketLocker() { close(fd); }

    std::string get_path() { return path; }

   private:
    std::string path{};
    int fd{-1};
};

// Ping-pong messages between server and client
TEST(UnixConnectionTests, SyncMessages)
{
    std::string client_to_server("testing this awesome comm library with some random data");
    std::string server_to_client("this awesome library seems to work :)");

    Barrier barrier(2);

    UnixSocketLocker unix_socket(UNIX_SOCKET_PATH_TEMPLATE);

    std::thread server_thread([&]() {
        auto config =
            comm::wrapConnServerConfig(new comm::UnixConnServerConfig(unix_socket.get_path()));
        comm::ConnServer server(comm::createConnList(config));

        barrier.wait();

        auto server_conn = server.negotiate_protocol(server.accept());

        for (int i = 0; i < NUMBER_OF_MESSAGES; ++i) {
            // Recieve something
            BytesBuffer message_received = server_conn->recv_message();
            std::string recv_message(reinterpret_cast< char* >(message_received.data()));
            ASSERT_EQ(0, recv_message.compare(client_to_server));

            // Send something
            server_conn->send_message(reinterpret_cast< const uint8_t* >(server_to_client.c_str()),
                                      server_to_client.length());
        }
    });

    comm::ConnClient conn_client({unix_socket.get_path(), 0, 1});

    barrier.wait();

    auto connection = conn_client.connect();

    for (int i = 0; i < NUMBER_OF_MESSAGES; ++i) {
        // Send something
        connection->send_message(reinterpret_cast< const uint8_t* >(client_to_server.c_str()),
                                 client_to_server.length());

        // Receive something
        BytesBuffer message_received = connection->recv_message();
        std::string recv_message(reinterpret_cast< char* >(message_received.data()));
        ASSERT_EQ(0, recv_message.compare(server_to_client));
    }

    server_thread.join();
}

// Both client and server send all messages first and then check the received messages.
TEST(UnixConnectionTests, AsyncMessages)
{
    std::string client_to_server("client sends some random data");
    std::string server_to_client("this library seems to work :)");

    Barrier barrier(2);
    UnixSocketLocker unix_socket(UNIX_SOCKET_PATH_TEMPLATE);

    std::thread server_thread([&]() {
        auto config =
            comm::wrapConnServerConfig(new comm::UnixConnServerConfig(unix_socket.get_path()));
        comm::ConnServer server(comm::createConnList(config));

        barrier.wait();

        auto server_conn = server.negotiate_protocol(server.accept());

        for (int i = 0; i < NUMBER_OF_MESSAGES; ++i) {
            // Send something
            server_conn->send_message(reinterpret_cast< const uint8_t* >(server_to_client.c_str()),
                                      server_to_client.length());
        }

        for (int i = 0; i < NUMBER_OF_MESSAGES; ++i) {
            // Recieve something
            BytesBuffer message_received = server_conn->recv_message();
            std::string recv_message(reinterpret_cast< char* >(message_received.data()));
            ASSERT_EQ(0, recv_message.compare(client_to_server));
        }
    });

    comm::ConnClient conn_client({unix_socket.get_path(), 0, 1});

    barrier.wait();

    auto connection = conn_client.connect();

    for (int i = 0; i < NUMBER_OF_MESSAGES; ++i) {
        // Send something
        connection->send_message(reinterpret_cast< const uint8_t* >(client_to_server.c_str()),
                                 client_to_server.length());
    }

    for (int i = 0; i < NUMBER_OF_MESSAGES; ++i) {
        // Receive something
        BytesBuffer message_received = connection->recv_message();
        std::string recv_message(reinterpret_cast< char* >(message_received.data()));
        ASSERT_EQ(0, recv_message.compare(server_to_client));
    }

    server_thread.join();
}

// Both client and server send all messages first and then check the received messages.
TEST(UnixConnectionTests, MultipleSequentialClients)
{
    std::string client_to_server("client sends some random data");
    std::string server_to_client("this library seems to work :)");

    Barrier barrier(2);
    UnixSocketLocker unix_socket(UNIX_SOCKET_PATH_TEMPLATE);

    std::thread server_thread([&]() {
        auto config =
            comm::wrapConnServerConfig(new comm::UnixConnServerConfig(unix_socket.get_path()));
        comm::ConnServer server(comm::createConnList(config));

        int executions = 0;
        while (++executions < NUMBER_OF_CLIENTS) {
            barrier.wait();

            auto server_conn = server.negotiate_protocol(server.accept());

            barrier.reset();

            for (int i = 0; i < NUMBER_OF_MESSAGES; ++i) {
                // Send something
                server_conn->send_message(
                    reinterpret_cast< const uint8_t* >(server_to_client.c_str()),
                    server_to_client.length());
            }

            for (int i = 0; i < NUMBER_OF_MESSAGES; ++i) {
                // Recieve something
                BytesBuffer message_received = server_conn->recv_message();
                std::string recv_message(reinterpret_cast< char* >(message_received.data()));
                ASSERT_EQ(0, recv_message.compare(client_to_server));
            }
        }
    });

    int client_exections = 0;
    while (++client_exections < NUMBER_OF_CLIENTS) {
        comm::ConnClient conn_client({unix_socket.get_path(), 0, 1});

        barrier.wait();

        auto connection = conn_client.connect();

        for (int i = 0; i < NUMBER_OF_MESSAGES; ++i) {
            // Send something
            connection->send_message(reinterpret_cast< const uint8_t* >(client_to_server.c_str()),
                                     client_to_server.length());
        }

        for (int i = 0; i < NUMBER_OF_MESSAGES; ++i) {
            // Receive something
            BytesBuffer message_received = connection->recv_message();
            std::string recv_message(reinterpret_cast< char* >(message_received.data()));
            ASSERT_EQ(0, recv_message.compare(server_to_client));
        }
    }

    server_thread.join();
}

// Server accepts connection and then goes down, client tries to recv.
TEST(UnixConnectionTests, ServerShutdownRecv)
{
    Barrier barrier(2);
    UnixSocketLocker unix_socket(UNIX_SOCKET_PATH_TEMPLATE);

    std::thread server_thread([&]() {
        auto config =
            comm::wrapConnServerConfig(new comm::UnixConnServerConfig(unix_socket.get_path()));
        comm::ConnServer server(comm::createConnList(config));

        barrier.wait();

        auto server_conn = server.negotiate_protocol(server.accept());
    });

    comm::ConnClient conn_client({unix_socket.get_path(), 0, 1});

    barrier.wait();

    auto connection = conn_client.connect();

    server_thread.join();  // Here the server will close the port.

    ASSERT_THROW(connection->recv_message(), comm::Exception);
}

TEST(UnixConnectionTests, SendArrayInts)
{
    int arr[10] = {22, 568, 254, 784, 452, 458, 235, 124, 1425, 1542};

    Barrier barrier(2);
    UnixSocketLocker unix_socket(UNIX_SOCKET_PATH_TEMPLATE);

    std::thread server_thread([&]() {
        auto config =
            comm::wrapConnServerConfig(new comm::UnixConnServerConfig(unix_socket.get_path()));
        comm::ConnServer server(comm::createConnList(config));

        barrier.wait();

        auto server_conn = server.negotiate_protocol(server.accept());

        server_conn->send_message(reinterpret_cast< const uint8_t* >(arr), sizeof(arr));
    });

    comm::ConnClient conn_client({unix_socket.get_path(), 0, 1});

    barrier.wait();

    auto connection = conn_client.connect();

    BytesBuffer message_received = connection->recv_message();

    const int* arr_recv = reinterpret_cast< const int* >(message_received.data());
    for (int i = 0; i < 10; ++i) {
        ASSERT_EQ(arr[i], arr_recv[i]);
    }

    server_thread.join();
}

TEST(UnixConnectionTests, MoveCopy)
{
    comm::UnixConnection a(comm::UnixSocket::create(), 0);
    comm::UnixConnection server_conn(comm::UnixSocket::create(), 0);
    server_conn = std::move(a);  // Testing copy with move works
}

TEST(UnixConnectionTests, ClientPathProblems)
{
    // TODO - ensure to choose a path that doesn't exit.
    std::string nonexistant_path = "/xyz/abc";
    comm::ConnClient client_1({nonexistant_path, 0, 1});

    ASSERT_THROW(client_1.connect(), comm::Exception);

    // create path too long for unix sockets.
    std::string too_long_path = "/";
    too_long_path.resize(109, 'x');
    comm::ConnClient client_2({too_long_path, 0, 1});

    ASSERT_THROW(client_2.connect(), comm::Exception);
}

TEST(UnixConnectionTests, ServerPathProblems)
{
    // TODO - ensure to choose a path that doesn't exit.
    std::string nonexistant_path = "/xyz/abc";

    auto config = comm::wrapConnServerConfig(new comm::UnixConnServerConfig(nonexistant_path));
    ASSERT_THROW(comm::ConnServer server_1(comm::createConnList(config)), comm::Exception);

    // create path too long for unix sockets.
    std::string too_long_path = "/";
    too_long_path.resize(109, 'x');
    comm::ConnClient client_2({too_long_path, 0, 1});

    auto config2 = comm::wrapConnServerConfig(new comm::UnixConnServerConfig(too_long_path));
    ASSERT_THROW(comm::ConnServer server_2(comm::createConnList(config2)), comm::Exception);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    // To make GoogleTest silent:
    // if (true) {
    //     auto& listeners = ::testing::UnitTest::GetInstance()->listeners();
    //     delete listeners.Release(listeners.default_result_printer());
    // }
    return RUN_ALL_TESTS();
}
