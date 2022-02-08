/**
 * @section LICENSE
 *
 * The MIT License
 *
 * @copyright Copyright (c) 2017 Intel Corporation
 * @copyright Copyright (c) 2021 ApertureData Inc
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

#include <chrono>
#include <memory>
#include <string>
#include <vector>

#include "aperturedb/VDMSClient.h"
#include "comm/Protocol.h"

namespace VDMS {

    struct AuthToken {
        std::chrono::system_clock::time_point issued_at;
        std::string refresh_token;
        int32_t refresh_token_expires_in{-1};
        std::string session_token;
        int32_t session_token_expires_in{-1};
    };

    class VDMSClientImpl : public TokenBasedVDMSClient {
    public:
        VDMSClientImpl(VDMSClientConfig config);
        ~VDMSClientImpl();

        // Blocking call
        VDMS::Response query(const std::string& json_query,
                             const std::vector<std::string*> blobs = {},
                             bool ignore_authentication = false);

    private:
        bool needs_re_authentication();
        bool needs_token_refresh();
        void re_authenticate();
        void refresh_token();

        VDMSClientConfig _config;
        std::unique_ptr<AuthToken> _auth_token;
    };
};
