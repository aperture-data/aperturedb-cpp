/**
 * @copyright Copyright (c) 2021 ApertureData Inc.
 */

#include <thread>

#include "gtest/gtest.h"

#include "PromConfig.h"
#include "prometheus_ambassador_defines.h"
#include "comm/Exception.h"


TEST(PromConfigTest, LoadConfig)
{
    auto cfg = PromConfig::load("test/test-config.json");
    EXPECT_EQ(cfg.prometheus_address, "0.0.0.0");
    EXPECT_EQ(cfg.prometheus_port, 8080);
    EXPECT_EQ(cfg.vdms_address, "aperturedb");
    EXPECT_EQ(cfg.vdms_port, 55557);
    EXPECT_EQ(cfg.username, "admin");
    EXPECT_EQ(cfg.password, "admin");
    EXPECT_TRUE(cfg.api_token.empty());
    EXPECT_EQ(cfg.protocols, VDMS::Protocol::TLS);

    const std::string ca_cert(R"(-----BEGIN CERTIFICATE-----
MIIDfTCCAmWgAwIBAgIUDMUB7YI1ybFb6FMWjwVHqBdIpf4wDQYJKoZIhvcNAQEL
BQAwTjELMAkGA1UEBhMCQUExCzAJBgNVBAgMAkFBMQswCQYDVQQHDAJBQTELMAkG
A1UECgwCQUExCzAJBgNVBAsMAkFBMQswCQYDVQQDDAJBQTAeFw0yMjAyMjQxNTAw
MDFaFw0yMjAyMjUxNTAwMDFaME4xCzAJBgNVBAYTAkFBMQswCQYDVQQIDAJBQTEL
MAkGA1UEBwwCQUExCzAJBgNVBAoMAkFBMQswCQYDVQQLDAJBQTELMAkGA1UEAwwC
QUEwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQC/XBhbsERpgvVVSz8T
BxAQTCqQvkeiMSUBfW79fysStdDhu+DljwyOd7vvIDzgg8qHXp12WvKRiQmqSUSw
MXZCPkMQiIR+uyWqFFaweuImWlYyjw5C+jw6GKuaN1JUvGGePcfb7Yex7swRYqvi
lipJqS+BYvahvFxN+/vHvoSp25l987WJh3lVXu78xmb+rsVjvEjoCGBt8e/mZDNS
g6OhApa94s3o/UTpMWr5XA5o8miY3JmSc8WUfvTz/PZ72h3xd4lB5ID4Rnin4mkS
RYiPSXyd4sf/COti7CW0ifbk2EI2XF+fRypdyZLbpdQkbWwF5ytsbvPfLMrkaYV1
rHzxAgMBAAGjUzBRMB0GA1UdDgQWBBS9InJfhRmFiXKpsQd9Kp938FAL6DAfBgNV
HSMEGDAWgBS9InJfhRmFiXKpsQd9Kp938FAL6DAPBgNVHRMBAf8EBTADAQH/MA0G
CSqGSIb3DQEBCwUAA4IBAQAOQqUnWn24dbQwPLKW1ZaN5XukWwOP/E+AksdFcZ4W
OcRez42mf/rnZNyR2BkoqRrYppDrfAnPCSovK0nV5dBtMPCZnRMTaxhym4rVQ3Ov
pXL/gSy/cB1OXXhWi0G/ncNzsAetDX3HKPf3bvFelpr5BW3ZS2iQW5y6pTLXej6J
9OieSyzCxpFuBxkP8042Il8KskmaWtbKthA7/7nkaYFbjFtnFbLz2wFfNuWTwLzG
09ioCr1SbxUJaFIvC3+6S7kfqtudP4Rhzjf4cMloxO8MwAWURv+691zEdhEAGCji
zg1yCw6s5NT+eoASa+HpBYoqUCsnUDB1+D/aT6H7oYiV
-----END CERTIFICATE-----
)");
    EXPECT_EQ(cfg.ca_certificate, ca_cert);
}

TEST(PromConfigTest, DefaultValues)
{
    PromConfig cfg("{}"_json);
    EXPECT_EQ(cfg.prometheus_address, PA_CONFIG_PROMETHEUS_ADDRESS_DEFAULT);
    EXPECT_EQ(cfg.prometheus_port, PA_CONFIG_PROMETHEUS_PORT_DEFAULT);
    EXPECT_EQ(cfg.vdms_address, PA_CONFIG_VDMS_ADDRESS_DEFAULT);
    EXPECT_EQ(cfg.vdms_port, PA_CONFIG_VDMS_PORT_DEFAULT);
    EXPECT_TRUE(cfg.username.empty());
    EXPECT_TRUE(cfg.password.empty());
    EXPECT_TRUE(cfg.api_token.empty());
    EXPECT_EQ(cfg.protocols, VDMS::Protocol::Any);
    EXPECT_TRUE(cfg.ca_certificate.empty());
}


TEST(PromConfigTest, BadCert)
{
    EXPECT_THROW(PromConfig("{\"ca_certificate\":\"file/that/does/not/exist\"}"_json), comm::Exception);
}


TEST(PromConfigTest, BadProtocols)
{
    EXPECT_THROW(PromConfig("{\"allowed_protocols\":\"foo\"}"_json), comm::Exception);
}
