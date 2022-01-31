/**
 *
 * @copyright Copyright (c) 2022 ApertureData Inc.
 *
 */

#include "PromConfig.h"

#include <fstream>
#include <iostream>
#include <aperturedb/VDMSClient.h>
#include "comm/Exception.h"

constexpr const char*    PromConfig::PROMETHEUS_ADDRESS_KEY;
constexpr const char*    PromConfig::PROMETHEUS_ADDRESS_DEFAULT;
constexpr const char*    PromConfig::PROMETHEUS_PORT_KEY;
constexpr int            PromConfig::PROMETHEUS_PORT_DEFAULT;
constexpr const char*    PromConfig::VDMS_ADDRESS_KEY;
constexpr const char*    PromConfig::VDMS_ADDRESS_DEFAULT;
constexpr const char*    PromConfig::VDMS_PORT_KEY;
constexpr int            PromConfig::VDMS_PORT_DEFAULT;
constexpr const char*    PromConfig::USERNAME_KEY;
constexpr const char*    PromConfig::PASSWORD_KEY;
constexpr const char*    PromConfig::API_TOKEN_KEY;
constexpr const char*    PromConfig::PROTOCOLS_KEY;
constexpr const char*    PromConfig::PROTOCOLS_DEFAULT;
constexpr const char*    PromConfig::CA_CERT_KEY;

PromConfig PromConfig::load(const std::string& config_file) {
    std::ifstream in_file(config_file);
    return PromConfig(nlohmann::json::parse(in_file));
}

namespace
{
    VDMS::Protocol parse_protocol(const std::string& val) {
        if (val == "tcp") return VDMS::Protocol::TCP;
        if (val == "tls") return VDMS::Protocol::TLS;
        if (val == "any") return VDMS::Protocol::Any;
        THROW_EXCEPTION(ProtocolError, "Invalid protocol in config");
    }
}

PromConfig::PromConfig(const nlohmann::json& config_json)
: prometheus_address(config_json.value(PROMETHEUS_ADDRESS_KEY, PROMETHEUS_ADDRESS_DEFAULT))
, prometheus_port(config_json.value(PROMETHEUS_PORT_KEY, PROMETHEUS_PORT_DEFAULT))
, vdms_address(config_json.value(VDMS_ADDRESS_KEY, VDMS_ADDRESS_DEFAULT))
, vdms_port(config_json.value(VDMS_PORT_KEY, VDMS_PORT_DEFAULT))
, username(config_json.value(USERNAME_KEY, std::string()))
, password(config_json.value(PASSWORD_KEY, std::string()))
, api_token(config_json.value(API_TOKEN_KEY, std::string()))
, protocols(parse_protocol(config_json.value(PROTOCOLS_KEY, PROTOCOLS_DEFAULT)))
, ca_certificate(config_json.value(CA_CERT_KEY, std::string()))
{}
