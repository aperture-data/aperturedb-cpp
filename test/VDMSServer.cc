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

#include "VDMSServer.h"

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

VDMSServer::VDMSServer(int port, comm::ConnServerConfig config) :
    _server(new comm::ConnServer(port, config))
{
    auto thread_function = [&]()
    {
        auto server_conn = _server->accept();

        while (!_stop_signal) {
            std::basic_string<uint8_t> message_received;

            try {
                message_received = server_conn->recv_message();
            } catch (comm::Exception exception) {
                if (exception.num == comm::ConnectionShutDown) {
                    return;
                }

                throw exception;
            }

            protobufs::queryMessage protobuf_response;
            protobuf_response.ParseFromArray(message_received.data(), message_received.length());

            protobufs::queryMessage cmd;
            cmd.set_json(protobuf_response.json());

            std::basic_string<uint8_t> msg(cmd.ByteSizeLong(), 0);
            cmd.SerializeToArray(const_cast<uint8_t*>(msg.data()), msg.length());

            server_conn->send_message(msg.data(), msg.length());
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
