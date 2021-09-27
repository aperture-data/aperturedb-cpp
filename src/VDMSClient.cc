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

#include "VDMSClient.h"

using namespace VDMS;

VDMSClient::VDMSClient(std::string addr,
                       int port,
                       comm::Protocol protocols,
                       std::string ca_certfificate) :
    TokenBasedVDMSClient(addr, port, protocols, ca_certfificate)
{
}

VDMSClient::VDMSClient(std::string /*username*/,
                       std::string /*password*/,
                       std::string addr,
                       int port,
                       comm::Protocol protocols,
                       std::string ca_certfificate) :
    VDMSClient(addr, port, protocols, ca_certfificate)
{
    // TODO: use the username and password to get a token
}

VDMSClient::VDMSClient(std::string /*api_key*/,
                       std::string addr,
                       int port,
                       comm::Protocol protocols,
                       std::string ca_certfificate) :
    VDMSClient(addr, port, protocols, ca_certfificate)
{
    // TODO: use the api key to get a token
}

VDMSClient::~VDMSClient() = default;

VDMS::Response VDMSClient::query(const std::string& json,
                                 const std::vector<std::string*> blobs)
{
    return TokenBasedVDMSClient::query(json, blobs, token);
}
