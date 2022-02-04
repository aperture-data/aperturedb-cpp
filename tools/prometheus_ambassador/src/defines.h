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
#define AD_METRIC_SCHEMA_STATUS       "status"
#define AD_METRIC_SCHEMA_VERSION      "version"
#define AD_METRIC_SCHEMA_VALUES       "values"
#define AD_METRIC_SCHEMA_FAMILIES     "families"
#define AD_METRIC_SCHEMA_NAME         "name"
#define AD_METRIC_SCHEMA_TYPE         "type"
#define AD_METRIC_SCHEMA_HELP         "help"
#define AD_METRIC_SCHEMA_METRICS      "metrics"
#define AD_METRIC_SCHEMA_LABELS       "labels"
#define AD_METRIC_SCHEMA_TIMESTAMP_MS "timestamp_ms"
#define AD_METRIC_SCHEMA_VALUE        "value"
#define AD_METRIC_SCHEMA_COUNT        "count"
#define AD_METRIC_SCHEMA_SUM          "sum"
#define AD_METRIC_SCHEMA_BUCKETS      "buckets"
#define AD_METRIC_SCHEMA_MAX          "max"
#define AD_METRIC_SCHEMA_QUANTILES    "quantiles"
#define AD_METRIC_SCHEMA_QUANTILE     "quantile"

#define PA_METRIC_CLIENT_CONNECTED_NAME    "prometheus_connected"
#define PA_METRIC_CLIENT_CONNECTED_HELP    "Whether the prometheus adapter is currently connected to the ApertureDB API"
#define PA_METRIC_CLIENT_FAILURES_NAME     "prometheus_failures_total"
#define PA_METRIC_CLIENT_FAILURES_HELP     "Communication failures observed by the prometheus adaptor attempting to connect via the ApertureDB API"
#define PA_METRIC_CLIENT_QUERIES_HELP      "Times the prometheus adaptor queried ApertureDB metrics"
#define PA_METRIC_CLIENT_QUERY_US_NAME     "prometheus_client_query_us"
#define PA_METRIC_CLIENT_QUERY_US_HELP     "Time taken by the prometheus adaptor to query metrics from ApertureDB, in microseconds"
#define PA_METRIC_CLIENT_QUERY_US_BUCKETS  {}

#define PA_METRIC_KEY_ADDRESS   "aperturedb_address"
#define PA_METRIC_KEY_MESSAGE   "message"
#define PA_METRIC_KEY_STAGE     "stage"
#define PA_METRIC_VALUE_CONNECT "connect"
#define PA_METRIC_VALUE_QUERY   "query"
#define PA_METRIC_VALUE_PARSE   "parse"
