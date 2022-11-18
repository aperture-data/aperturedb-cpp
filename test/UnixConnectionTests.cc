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

#define NUMBER_OF_MESSAGES      20

typedef std::basic_string< uint8_t > BytesBuffer;

// Ping-pong messages between server and client
TEST(UnixConnectionTests, SyncMessages)
{
    std::string client_to_server("testing this awesome comm library with some random data");
    std::string server_to_client("this awesome library seems to work :)");

    Barrier barrier(2);

    std::string tmp_unix_path { "/tmp/aperturedb-unix-socket.XXXXXX" };
    int tmp_fd = mkstemp( tmp_unix_path.data() );

    std::cout << "Temp unix path is " << tmp_unix_path << "\n";
    unlink( tmp_unix_path.c_str() );
    close( tmp_fd );


    std::thread server_thread([&]() {
    //: _server( createConnList( wrapConnServerConfig( config.connServerConfig.addPort(port) ) ) ) 
        auto config = comm::wrapConnServerConfig( new comm::UnixConnServerConfig( tmp_unix_path ));
        comm::ConnServer server( comm::createConnList( config ));
	std::cout << "Server Init\n";

        barrier.wait();
	std::cout << "Server Running\n";

        auto server_conn = server.negotiate_protocol(server.accept());
	std::cout << "Server Attached:\n";

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

	std::cout << "Client Start\n";
    comm::ConnClient conn_client({tmp_unix_path,0,1});
	std::cout << "Client Init\n";

    barrier.wait();
	std::cout << "Client Run\n";

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

