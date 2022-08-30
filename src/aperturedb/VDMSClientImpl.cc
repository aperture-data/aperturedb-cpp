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

#include "VDMSClientImpl.h"

#include <chrono>
#include <nlohmann/json.hpp>

#include "comm/Exception.h"

using namespace VDMS;

namespace
{

std::unique_ptr< AuthToken > process_token_response(const std::string& response,
                                                    const std::string& object_name)
{
    auto auth_token = std::unique_ptr< AuthToken >(new AuthToken());

    auth_token->issued_at = std::chrono::system_clock::now();

    auto responseJson = nlohmann::json::parse(response);

    if (responseJson.is_array() && responseJson.size() == 1) {
        auto responseElement = responseJson.at(0);

        if (responseElement.is_object() && responseElement.contains(object_name)) {
            auto authenticateElement = responseElement[object_name];

            if (authenticateElement.is_object() && authenticateElement.contains("status")) {
                if (authenticateElement["status"] == 0) {
                    if (authenticateElement.contains("refresh_token")) {
                        auth_token->refresh_token = authenticateElement["refresh_token"];
                    }

                    if (authenticateElement.contains("refresh_token_expires_in")) {
                        auth_token->refresh_token_expires_in =
                            authenticateElement["refresh_token_expires_in"].get< int32_t >();
                    }

                    if (authenticateElement.contains("session_token")) {
                        auth_token->session_token = authenticateElement["session_token"];
                    }

                    if (authenticateElement.contains("session_token_expires_in")) {
                        auth_token->session_token_expires_in =
                            authenticateElement["session_token_expires_in"].get< int32_t >();
                    }
                }
            }
        }
    }

    if (auth_token->session_token.empty()) {
        THROW_EXCEPTION(AuthenticationError);
    }

    return auth_token;
}

}  // namespace

VDMSClientImpl::VDMSClientImpl(std::string username,
                               std::string password,
                               const VDMSClientConfig& config)
    : TokenBasedVDMSClient(config)
    , _username(std::move(username))
    , _password(std::move(password))
    , _api_key()
    , _auth_token()
{
    re_authenticate();
}

VDMSClientImpl::VDMSClientImpl(std::string api_key, const VDMSClientConfig& config)
    : TokenBasedVDMSClient(config)
    , _username()
    , _password()
    , _api_key(std::move(api_key))
    , _auth_token()
{
    re_authenticate();
}

VDMSClientImpl::~VDMSClientImpl() = default;

bool VDMSClientImpl::needs_re_authentication()
{
    auto expiration_point =
        _auth_token->issued_at + std::chrono::seconds(_auth_token->refresh_token_expires_in);
    auto now = std::chrono::system_clock::now();

    return expiration_point <= now;
}

bool VDMSClientImpl::needs_token_refresh()
{
    auto expiration_point =
        _auth_token->issued_at + std::chrono::seconds(_auth_token->session_token_expires_in);
    auto now = std::chrono::system_clock::now();

    return expiration_point <= now;
}

VDMS::Response VDMSClientImpl::query(const std::string& json,
                                     const std::vector< std::string* > blobs,
                                     bool ignore_authentication)
{
    if (_auth_token && !ignore_authentication) {
        if (needs_re_authentication()) {
            re_authenticate();
        }

        if (needs_token_refresh()) {
            refresh_token();
        }

        return TokenBasedVDMSClient::query(json, blobs, _auth_token->session_token);
    }

    return TokenBasedVDMSClient::query(json, blobs);
}

void VDMSClientImpl::re_authenticate()
{
    nlohmann::json requestJson;

    if (!_username.empty()) {
        requestJson = nlohmann::json::array(
            {{{"Authenticate", {{"username", _username}, {"password", _password}}}}});
    } else {
        requestJson = nlohmann::json::array({{{"Authenticate", {{"token", _api_key}}}}});
    }

    auto response = query(requestJson.dump(), {}, true);

    _auth_token = process_token_response(response.json, "Authenticate");
}

void VDMSClientImpl::refresh_token()
{
    auto requestJson = nlohmann::json::array(
        {{{"RefreshToken", {{"refresh_token", _auth_token->refresh_token}}}}});

    auto response = query(requestJson.dump(), {}, true);

    _auth_token = process_token_response(response.json, "RefreshToken");
}
