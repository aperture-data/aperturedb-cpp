/**
 *
 * @copyright Copyright (c) 2022 ApertureData Inc.
 *
 */

#pragma once


// Config
#define PA_CONFIG_PROMETHEUS_ADDRESS_KEY        "prometheus_address"
#define PA_CONFIG_PROMETHEUS_ADDRESS_DEFAULT    "localhost"
#define PA_CONFIG_PROMETHEUS_PORT_KEY           "prometheus_port"
#define PA_CONFIG_PROMETHEUS_PORT_DEFAULT       8080
#define PA_CONFIG_VDMS_ADDRESS_KEY              "vdms_address"
#define PA_CONFIG_VDMS_ADDRESS_DEFAULT          "localhost"
#define PA_CONFIG_VDMS_PORT_KEY                 "vdms_port"
#define PA_CONFIG_VDMS_PORT_DEFAULT             VDMS::VDMS_PORT
#define PA_CONFIG_USERNAME_KEY                  "username"
#define PA_CONFIG_USERNAME_DEFAULT              ""
#define PA_CONFIG_PASSWORD_KEY                  "password"
#define PA_CONFIG_PASSWORD_DEFAULT              ""
#define PA_CONFIG_API_TOKEN_KEY                 "api_token"
#define PA_CONFIG_API_TOKEN_DEFAULT             ""
#define PA_CONFIG_PROTOCOLS_KEY                 "allowed_protocols"
#define PA_CONFIG_PROTOCOLS_DEFAULT             "any"
#define PA_CONFIG_CA_CERT_KEY                   "ca_certificate"
#define PA_CONFIG_CA_CERT_DEFAULT               ""

// Metrics
#define PA_METRIC_CLIENT_CONNECTED_NAME             "prometheus_connected"
#define PA_METRIC_CLIENT_CONNECTED_HELP             "Whether the prometheus adapter is currently connected to the ApertureDB API"
#define PA_METRIC_CLIENT_FAILURES_NAME              "prometheus_failures_total"
#define PA_METRIC_CLIENT_FAILURES_HELP              "Communication failures observed by the prometheus adaptor attempting to connect via the ApertureDB API"
#define PA_METRIC_CLIENT_QUERIES_HELP               "Times the prometheus adaptor queried ApertureDB metrics"
#define PA_METRIC_CLIENT_QUERY_SECONDS_NAME         "prometheus_client_query_seconds"
#define PA_METRIC_CLIENT_QUERY_SECONDS_HELP         "Time taken by the prometheus adaptor to query metrics from ApertureDB, in microseconds"
#define PA_METRIC_CLIENT_QUERY_SECONDS_QTILES       {{0.5,0.05},{0.9,0.05},{0.99,0.05}}
#define PA_METRIC_CLIENT_BYTES_TRANSFERRED_NAME     "prometheus_client_bytes_received"
#define PA_METRIC_CLIENT_BYTES_TRANSFERRED_HELP     "Bytes received by the prometheus adaptor"
#define PA_METRIC_CLIENT_BYTES_TRANSFERRED_BUCKETS  {}

#define PA_METRIC_KEY_ADDRESS   "aperturedb_address"
#define PA_METRIC_KEY_MESSAGE   "message"
#define PA_METRIC_KEY_STAGE     "stage"
#define PA_METRIC_VALUE_CONNECT "connect"
#define PA_METRIC_VALUE_QUERY   "query"
#define PA_METRIC_VALUE_PARSE   "parse"
#define PA_METRIC_KEY_DIRECTION "direction"
#define PA_METRIC_VALUE_SENT    "sent"
#define PA_METRIC_VALUE_RECV    "received"
