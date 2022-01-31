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

#define PA_METRIC_CLIENT_CONNECTIONS_NAME       "client_connections"
#define PA_METRIC_CLIENT_CONNECTIONS_HELP       "Times the prometheus adaptor attempted to connect to the ApertureDB API"
#define PA_METRIC_CLIENT_QUERIES_NAME           "client_queries"
#define PA_METRIC_CLIENT_QUERIES_HELP           "Times the prometheus adaptor queried ApertureDB metrics"
#define PA_METRIC_CLIENT_QUERY_SECONDS_NAME     "client_query_seconds"
#define PA_METRIC_CLIENT_QUERY_SECONDS_HELP     "Time taken by the prometheus adaptor to query metrics from ApertureDB, in seconds"
#define PA_METRIC_CLIENT_QUERY_SECONDS_BUCKETS  {0.001, 0.01, 0.1, 1.0, 10.0}

#define PA_METRIC_KEY_HOST      "host"
#define PA_METRIC_KEY_PORT      "port"
#define PA_METRIC_KEY_RESULT    "result"
#define PA_METRIC_VALUE_SUCCESS "success"
#define PA_METRIC_VALUE_FAILURE "failure"
#define PA_METRIC_KEY_DETAILS   "details"