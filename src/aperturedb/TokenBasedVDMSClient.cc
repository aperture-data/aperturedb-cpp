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

#include "aperturedb/VDMSClient.h"
#include "aperturedb/queryMessageWrapper.h"
#include "aperturedb/Exception.h"
#include "comm/ConnClient.h"
#include "comm/Connection.h"
#include "comm/Exception.h"

using namespace VDMS;

TokenBasedVDMSClient::TokenBasedVDMSClient(const VDMSClientConfig& config)
    : _client(new comm::ConnClient(
          {config.addr, config.port},
          comm::ConnClientConfig(config.protocols, config.ca_certificate, false, config.metrics)))
    , _connection(_client->connect())
{
}

TokenBasedVDMSClient::~TokenBasedVDMSClient() = default;

VDMS::Response TokenBasedVDMSClient::query(const std::string& json,
                                           const std::vector< std::string* > blobs,
                                           const std::string& token)
{
    try {
        protobufs::queryMessage cmd;
        cmd.set_json(json);
        cmd.set_token(token);

        for (auto& it : blobs) {
            std::string* blob = cmd.add_blobs();
            *blob             = *it;
        }

        std::basic_string< uint8_t > msg(cmd.ByteSizeLong(), 0);
        cmd.SerializeToArray(msg.data(), msg.length());

        _connection->send_message(msg.data(), msg.length());

        // Wait for response (blocking call)
        msg = _connection->recv_message();

        protobufs::queryMessage protobuf_response;
        protobuf_response.ParseFromArray(msg.data(), msg.length());

        VDMS::Response response;
        response.json = protobuf_response.json();

        for (auto& it : protobuf_response.blobs()) {
            response.blobs.push_back(it);
        }

        return response;
    } catch (const comm::Exception& e) {
        throw VDMS::Exception(e.num, e.name, e.errno_val, e.msg, e.file, e.line);
    }
}
