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

#include <chrono>

#include "aperturedb/Exception.h"
#include "aperturedb/VDMSClientImpl.h"
#include "comm/Exception.h"

using namespace VDMS;

VDMSClient::VDMSClient(std::string addr,
                       int port,
                       Protocol protocols,
                       std::string ca_certificate) :
    _impl(new VDMSClientImpl(std::move(addr), port, static_cast<comm::Protocol>(protocols), std::move(ca_certificate)))
{
}

VDMSClient::VDMSClient(std::string username,
                       std::string password,
                       std::string addr,
                       int port,
                       Protocol protocols,
                       std::string ca_certificate) :
    _impl(new VDMSClientImpl(std::move(username), std::move(password), std::move(addr), port, static_cast<comm::Protocol>(protocols), std::move(ca_certificate)))
{
}

VDMSClient::VDMSClient(std::string api_key,
                       std::string addr,
                       int port,
                       Protocol protocols,
                       std::string ca_certificate) :
    _impl(new VDMSClientImpl(std::move(api_key), std::move(addr), port, static_cast<comm::Protocol>(protocols), std::move(ca_certificate)))
{
}

VDMSClient::~VDMSClient() = default;

VDMS::Response VDMSClient::query(const std::string& json,
                                 const std::vector<std::string*> blobs)
{
    try {
        return _impl->query(json, blobs);
    }
    // Addin this try-catch just for consistency
    // In the current implementation this catch should never be hit
    // as the comm::Exception is rethown by TokenBasedVDMSClient::query
    catch (const comm::Exception& e) {
        throw VDMS::Exception(e.num, e.name, e.errno_val, e.msg, e.file, e.line);
    }
}
