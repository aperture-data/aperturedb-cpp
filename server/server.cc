
#include <chrono>
#include <string>
#include <thread>
#include <iostream>

#include "Certificates.h"
#include "ConnClient.h"
#include "ConnServer.h"
#include "ExceptionComm.h"

#define SERVER_PORT_INTERCHANGE 43444
// #define SERVER_PORT_MULTIPLE    43444
// #define NUMBER_OF_MESSAGES 20

typedef std::basic_string<uint8_t> BytesBuffer;

int start_server();

// Server is set to accept any connection (TLS or just TCP)
static const comm::ConnServerConfig connServerConfig {
            comm::Protocol::TLS | comm::Protocol::TCP,
            ca_certificate,
            server_certificate,
            server_private_key
        };

int start_server()
{
    std::string server_to_client("Cool, got you, bro.");

    comm::ConnServer server(SERVER_PORT_INTERCHANGE, connServerConfig);
    std::cout << "===============================================" << std::endl;
    std::cout << "Server is listening..." << std::endl;

    try {
        auto server_conn = server.accept();

        std::cout << "Server has accepted an incoming connection." << std::endl;

        while(true) {

            //Recieve something
            BytesBuffer message_received = server_conn->recv_message();
            std::string recv_message(reinterpret_cast<char*>(const_cast<uint8_t*>(message_received.data())));
            std::cout << "Server Recieved the following message:" << std::endl;
            std::cout << recv_message << std::endl;

            if (recv_message == "stop"){
                std::cout << "Client wants me to die :( ..." << std::endl;
                break;
            }

            std::cout << "Server will send a quick response." << std::endl;
            //Send something
            server_conn->send_message(reinterpret_cast<const uint8_t*>(server_to_client.c_str()),
                                     server_to_client.length());
            std::cout << "Message sent correctly" << std::endl;
        }
    }
    catch (const comm::Exception& e){
        std::cout << "Exception: " << e.msg << std::endl;
        print_exception(e);
    }

    std::cout << "Shutting down server. Bye." << std::endl;

    return 0;
}

int main()
{
    while(true) {
        start_server();
    }

    return 0;
}
