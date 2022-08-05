/**
 * @copyright Copyright (c) 2021 ApertureData Inc.
 */

#include "VDMSServer.h"
#include "aperturedb/queryMessageWrapper.h"
#include "comm/Connection.h"
#include "comm/ConnServer.h"
#include "comm/Exception.h"

using namespace VDMS;

VDMSServer::VDMSServer(int port, comm::ConnServerConfig config) :
    _server(new comm::ConnServer(port, config))
{
    auto thread_function = [&]()
    {
        auto server_conn = _server.negotiate_protocol(_server.accept());

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
            protobuf_response.set_json(protobuf_request.json());

            send_message(server_conn, protobuf_response);
        }
    };

    _work_thread = std::unique_ptr<std::thread>(new std::thread(thread_function));
}

VDMSServer::~VDMSServer()
{
    _stop_signal = true;

    if (_work_thread->joinable()) {
        _work_thread->join();
    }
}

protobufs::queryMessage VDMSServer::receive_message(const std::shared_ptr<comm::Connection>& connection)
{
    std::basic_string<uint8_t> message_received = connection->recv_message();

    protobufs::queryMessage protobuf_request;
    bool ok = protobuf_request.ParseFromArray(message_received.data(), message_received.length());

    if (!ok) {
        throw std::runtime_error("Error parsing response using protobuf message\n");
    }

    return protobuf_request;
}

void VDMSServer::send_message(const std::shared_ptr<comm::Connection>& connection,
                              const protobufs::queryMessage& protobuf_response)
{
    std::basic_string<uint8_t> message(protobuf_response.ByteSizeLong(), 0);
    protobuf_response.SerializeToArray(message.data(), message.length());

    connection->send_message(message.data(), message.length());
}
