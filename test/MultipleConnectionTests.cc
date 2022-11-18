/**
 * @copyright Copyright (c) 2022 ApertureData Inc.
 */

#include <string>
#include <thread>
#include <stdlib.h>

#include "gtest/gtest.h"

#include "Barrier.h"
#include "comm/ConnClient.h"
#include "comm/ConnServer.h"
#include "comm/Exception.h"
#include "comm/TCPConnection.h"
#include "comm/UnixConnection.h"

#define SERVER_PORT_INTERCHANGE 43444
//#define SERVER_PORT_MULTIPLE    43444
#define NUMBER_OF_MESSAGES 20
//#define NUMBER_OF_CLIENTS  4

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
        unlink(path.c_str());
        // close( tmp_fd );
        fd = tmp_fd;
    }
    ~UnixSocketLocker() { close(fd); }

    std::string get_path() { return path; }

   private:
    std::string path{};
    int fd{-1};
};

// Ping-pong messages between server and client
TEST(MultipleConnectionTests, SequentialSyncMessages)
{
    std::string client_to_server("testing this awesome comm library with some random data");
    std::string server_to_client("this awesome library seems to work :)");

    Barrier barrier(2);

    int client_count = 2;

    UnixSocketLocker unix_socket(UNIX_SOCKET_PATH_TEMPLATE);

    std::thread server_thread([&]() {
        auto unix_config =
            comm::wrapConnServerConfig(new comm::UnixConnServerConfig(unix_socket.get_path()));
        auto tcp_config =
            comm::wrapConnServerConfig(new comm::TCPConnServerConfig(SERVER_PORT_INTERCHANGE));
        comm::ConnServer server(comm::createConnList(unix_config, tcp_config));
        // comm::ConnServer server(comm::createConnList(unix_config));
        std::cout << "Server Init\n";

        int executions = 0;
        while (++executions < client_count + 1) {
            std::cout << "Server Wait\n";
            barrier.wait();
            std::cout << "Server Running\n";

            auto server_conn = server.negotiate_protocol(server.accept());
            barrier.reset();
            std::cout << "Server Attached:\n";

            for (int i = 0; i < NUMBER_OF_MESSAGES; ++i) {
                // Recieve something
                std::cout << "Server Transmitting\n";
                BytesBuffer message_received = server_conn->recv_message();
                std::string recv_message(reinterpret_cast< char* >(message_received.data()));
                ASSERT_EQ(0, recv_message.compare(client_to_server));

                // Send something
                server_conn->send_message(
                    reinterpret_cast< const uint8_t* >(server_to_client.c_str()),
                    server_to_client.length());
            }
            std::cout << "Server Done\n";
        }
        std::cout << "Server Complete\n";
    });

    std::vector< std::unique_ptr< comm::ConnClient > > clients;

    clients.push_back(
        std::unique_ptr< comm ::ConnClient >(new comm::ConnClient({unix_socket.get_path(), 0, 1})));
    clients.push_back(std::unique_ptr< comm ::ConnClient >(
        new comm::ConnClient({"localhost", SERVER_PORT_INTERCHANGE})));

    for (auto& client : clients) {
        std::cout << "Client Start\n";
        std::cout << "Client Init\n";

        barrier.wait();
        std::cout << "Client Run\n";

        auto connection = client->connect();

        for (int i = 0; i < NUMBER_OF_MESSAGES; ++i) {
            // Send something
            connection->send_message(reinterpret_cast< const uint8_t* >(client_to_server.c_str()),
                                     client_to_server.length());

            // Receive something
            BytesBuffer message_received = connection->recv_message();
            std::string recv_message(reinterpret_cast< char* >(message_received.data()));
            ASSERT_EQ(0, recv_message.compare(server_to_client));
        }
        std::cout << "Client Done\n";
        // close connection
        client.reset();
        std::cout << "Client Closed\n";
    }

    server_thread.join();
}

