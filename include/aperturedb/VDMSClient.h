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

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Protocol.h"

namespace comm {

    class ConnClient;
    class Connection;

}

namespace VDMS {

    class VDMSClientImpl;

    static const int VDMS_PORT = 55555;

    struct Response {
        std::string json{};
        std::vector<std::string> blobs{};
    };

    class TokenBasedVDMSClient {
        // The constructor of the ConnClient class already connects to the
        // server if instantiated with the right address and port and it gets
        // disconnected when the class goes out of scope. For now, we
        // will leave the functioning like that. If the client has a need to
        // disconnect and connect specifically, then we can add explicit calls.
        std::unique_ptr<comm::ConnClient> _client;
        std::shared_ptr<comm::Connection> _connection;

    public:
        TokenBasedVDMSClient(std::string addr = "localhost",
                             int port = VDMS_PORT,
                             comm::Protocol protocols = comm::Protocol::TCP | comm::Protocol::TLS,
                             std::string ca_certfificate = "");
        ~TokenBasedVDMSClient();

        // Blocking call
        VDMS::Response query(const std::string& json_query,
                             const std::vector<std::string*> blobs = {},
                             const std::string& token = "");
    };

    class VDMSClient {
    public:
        VDMSClient(std::string addr = "localhost",
                   int port = VDMS_PORT,
                   comm::Protocol protocols = comm::Protocol::TCP | comm::Protocol::TLS,
                   std::string ca_certfificate = "");
        VDMSClient(std::string username,
                   std::string password,
                   std::string addr = "localhost",
                   int port = VDMS_PORT,
                   comm::Protocol protocols = comm::Protocol::TCP | comm::Protocol::TLS,
                   std::string ca_certfificate = "");
        VDMSClient(std::string api_key,
                   std::string addr = "localhost",
                   int port = VDMS_PORT,
                   comm::Protocol protocols = comm::Protocol::TCP | comm::Protocol::TLS,
                   std::string ca_certfificate = "");
        ~VDMSClient();

        // Blocking call
        VDMS::Response query(const std::string& json_query,
                             const std::vector<std::string*> blobs = {});

    private:
        std::unique_ptr<VDMSClientImpl> _impl;
    };
};
