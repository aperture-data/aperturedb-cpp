/**
 *
 * @copyright Copyright (c) 2022 ApertureData Inc.
 *
 */

#pragma once

#include <aperturedb/VDMSClient.h>
#include <nlohmann/json.hpp>

struct PromConfig {
    static constexpr const char*    PROMETHEUS_ADDRESS_KEY      { "prometheus_address" };
    static constexpr const char*    PROMETHEUS_ADDRESS_DEFAULT  { "localhost" };

    static constexpr const char*    PROMETHEUS_PORT_KEY         { "prometheus_port" };
    static constexpr int            PROMETHEUS_PORT_DEFAULT     { 8080 };

    static constexpr const char*    VDMS_ADDRESS_KEY            { "vdms_address" };
    static constexpr const char*    VDMS_ADDRESS_DEFAULT        { "localhost" };

    static constexpr const char*    VDMS_PORT_KEY               { "vdms_port" };
    static constexpr int            VDMS_PORT_DEFAULT           { VDMS::VDMS_PORT };

    static constexpr const char*    USERNAME_KEY                { "username" };

    static constexpr const char*    PASSWORD_KEY                { "password" };

    static constexpr const char*    API_TOKEN_KEY               { "api_token" };

    static constexpr const char*    PROTOCOLS_KEY               { "allowed_protocols" };
    static constexpr const char*    PROTOCOLS_DEFAULT           { "any" };

    static constexpr const char*    CA_CERT_KEY                 { "ca_certificate" };

    std::string prometheus_address;
    int prometheus_port;
    std::string vdms_address;
    int vdms_port;
    std::string username;
    std::string password;
    std::string api_token;
    VDMS::Protocol protocols;
    std::string ca_certificate;

    static PromConfig load(const std::string& config_file);
private:
    PromConfig(const nlohmann::json& config_json);
};
