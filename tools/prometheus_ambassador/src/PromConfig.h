/**
 *
 * @copyright Copyright (c) 2022 ApertureData Inc.
 *
 */

#pragma once

#include <aperturedb/VDMSClient.h>
#include <nlohmann/json.hpp>

struct PromConfig {
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
    PromConfig(const nlohmann::json& config_json);
};
