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

#include <chrono>
#include <string>
#include <thread>

#include "gtest/gtest.h"

#include "Certificates.h"
#include "ConnServer.h"
#include "ExceptionComm.h"
#include "VDMSClient.h"
#include "VDMSServer.h"

#define SERVER_PORT_INTERCHANGE 43444
#define NUMBER_OF_MESSAGES 20

typedef std::basic_string<uint8_t> BytesBuffer;

static const comm::ConnServerConfig connServerConfig{comm::Protocol::TLS, ca_certificate, server_certificate, server_private_key};

// Ping-pong messages between server and client
TEST(VDMSServerTests, SyncMessages)
{
    std::string client_to_server("testing this awesome comm library with " \
                                 "come random data");

    VDMS::VDMSServer server(SERVER_PORT_INTERCHANGE, connServerConfig);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    VDMS::VDMSClient client("localhost", SERVER_PORT_INTERCHANGE, comm::Protocol::TLS, ca_certificate);

    for (int i = 0; i < NUMBER_OF_MESSAGES; ++i) {
        // Send a query
        auto response = client.query(client_to_server.c_str());

        // Expect the same response
        ASSERT_EQ(0, response.json.compare(client_to_server));
    }
}
