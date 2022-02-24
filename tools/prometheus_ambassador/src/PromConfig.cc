/**
 *
 * @copyright Copyright (c) 2022 ApertureData Inc.
 *
 */

#include "PromConfig.h"
#include "prometheus_ambassador_defines.h"

#include <fstream>
#include <iostream>
#include "comm/Exception.h"

PromConfig PromConfig::load(const std::string& config_file) {
    std::ifstream in_file(config_file);
    return PromConfig(nlohmann::json::parse(in_file, nullptr, true, true));
}

namespace
{
    VDMS::Protocol parse_protocol(const std::string& val) {
        if (val == "tcp") return VDMS::Protocol::TCP;
        if (val == "tls") return VDMS::Protocol::TLS;
        if (val == "any") return VDMS::Protocol::Any;
        THROW_EXCEPTION(ProtocolError, "Invalid protocol in config");
    }

    std::string load_cert(std::string path) {
        std::string out;
        if ( !path.empty() ) {
            std::ifstream ifs(path);
            using _it_type = std::istreambuf_iterator<char>;
            out.assign( _it_type(ifs), _it_type() );
        }
        return out;
    }
}

PromConfig::PromConfig(const nlohmann::json& config_json)
: prometheus_address(config_json.value(PA_CONFIG_PROMETHEUS_ADDRESS_KEY, PA_CONFIG_PROMETHEUS_ADDRESS_DEFAULT))
, prometheus_port(config_json.value(PA_CONFIG_PROMETHEUS_PORT_KEY, PA_CONFIG_PROMETHEUS_PORT_DEFAULT))
, vdms_address(config_json.value(PA_CONFIG_VDMS_ADDRESS_KEY, PA_CONFIG_VDMS_ADDRESS_DEFAULT))
, vdms_port(config_json.value(PA_CONFIG_VDMS_PORT_KEY, PA_CONFIG_VDMS_PORT_DEFAULT))
, username(config_json.value(PA_CONFIG_USERNAME_KEY, PA_CONFIG_USERNAME_DEFAULT))
, password(config_json.value(PA_CONFIG_PASSWORD_KEY, PA_CONFIG_PASSWORD_DEFAULT))
, api_token(config_json.value(PA_CONFIG_API_TOKEN_KEY, PA_CONFIG_API_TOKEN_DEFAULT))
, protocols(parse_protocol(config_json.value(PA_CONFIG_PROTOCOLS_KEY, PA_CONFIG_PROTOCOLS_DEFAULT)))
, ca_certificate(load_cert(config_json.value(PA_CONFIG_CA_CERT_KEY, PA_CONFIG_CA_CERT_DEFAULT)))
{}
