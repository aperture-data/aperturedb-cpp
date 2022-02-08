/**
 *
 * @copyright Copyright (c) 2022 ApertureData Inc.
 *
 */

#include <thread>
#include <iostream>
#include "ClientCollector.h"
#include "prometheus_ambassador_defines.h"
#include "metrics/Timer.h"
#include "metrics/JsonReader.h"
#include "PrintCaughtException.h"

void ClientCollector::connect() const {
    _client.reset(new VDMS::VDMSClient(VDMS::VDMSClientConfig(
        config.vdms_address,
        config.vdms_port,
        config.protocols,
        config.ca_certificate,
        config.username,
        config.password,
        config.api_token,
        &_metrics
    )));
    std::cout << "Prometheus ambassador connected to "
        << config.vdms_address << ":" << config.vdms_port << std::endl;
}

ClientCollector::ClientCollector(const PromConfig& cfg, prometheus::Registry& registry)
: config(cfg)
, _client(nullptr)
, _registry(registry)
, _metrics(config, _registry)
{
}

std::vector<prometheus::MetricFamily> ClientCollector::Collect() const {
    const auto query = R"(
        [{"GetMetrics": {
            "format": "json"
        }}]
    )"_json;

    try {
        metrics::Timer<std::chrono::seconds, prometheus::Summary> timer;
        if (!_client) {
            timer.reset(&_metrics.connect_timer);
            connect();
        }

        _metrics.client_connected.Set(1.);
        timer.reset(&_metrics.query_timer);

        auto res = _client->query(query.dump());

        timer.reset(&_metrics.parse_timer);

        auto response = nlohmann::json::parse(res.json);
        if (response.is_array()) {
            metrics::JsonReader<nlohmann::json> reader;
            return reader.parse_metrics(response[0]["GetMetrics"][AD_METRIC_SCHEMA_VALUES]);
        }
        _metrics.increment_failure(std::string("Unexpected response: ") + response.dump());
    }
    catch (...) {
        _metrics.increment_failure(print_caught_exception());
    }

    // failed
    if (_client) {
        std::cout << "Client connection closed" << std::endl;
        _client.reset();
    }
    _metrics.client_connected.Set(0.);
    return {};
}

ClientCollector::Metrics::Metrics(const PromConfig& config, prometheus::Registry& registry)
: _static_labels({
    {PA_METRIC_KEY_ADDRESS, config.vdms_address + ":" + std::to_string(config.vdms_port)}
})
, _client_connected(prometheus::BuildGauge()
    .Name(PA_METRIC_CLIENT_CONNECTED_NAME)
    .Help(PA_METRIC_CLIENT_CONNECTED_HELP)
    .Labels(_static_labels)
    .Register(registry))
, _failures_total(prometheus::BuildCounter()
    .Name(PA_METRIC_CLIENT_FAILURES_NAME)
    .Help(PA_METRIC_CLIENT_FAILURES_HELP)
    .Labels(_static_labels)
    .Register(registry))
, _client_query_sec(prometheus::BuildSummary()
    .Name(PA_METRIC_CLIENT_QUERY_SECONDS_NAME)
    .Help(PA_METRIC_CLIENT_QUERY_SECONDS_HELP)
    .Labels(_static_labels)
    .Register(registry))
, _client_query_quantiles(PA_METRIC_CLIENT_QUERY_SECONDS_QTILES)
, _bytes_transferred(prometheus::BuildHistogram()
    .Name(PA_METRIC_CLIENT_BYTES_TRANSFERRED_NAME)
    .Help(PA_METRIC_CLIENT_BYTES_TRANSFERRED_HELP)
    .Labels(_static_labels)
    .Register(registry))
, _bytes_transferred_buckets(PA_METRIC_CLIENT_BYTES_TRANSFERRED_BUCKETS)
, client_connected(_client_connected.Add({}))
, connect_timer(_client_query_sec.Add({
    {PA_METRIC_KEY_STAGE, PA_METRIC_VALUE_CONNECT}
}, _client_query_quantiles, std::chrono::hours{1}))
, query_timer(_client_query_sec.Add({
    {PA_METRIC_KEY_STAGE, PA_METRIC_VALUE_QUERY}
}, _client_query_quantiles, std::chrono::hours{1}))
, parse_timer(_client_query_sec.Add({
    {PA_METRIC_KEY_STAGE, PA_METRIC_VALUE_PARSE}
}, _client_query_quantiles, std::chrono::hours{1}))
, bytes_sent(_bytes_transferred.Add({
    {PA_METRIC_KEY_DIRECTION, PA_METRIC_VALUE_SENT}
}, _bytes_transferred_buckets))
, bytes_recv(_bytes_transferred.Add({
    {PA_METRIC_KEY_DIRECTION, PA_METRIC_VALUE_RECV}
}, _bytes_transferred_buckets))
{
}

void ClientCollector::Metrics::increment_failure(const std::string& msg) {
    auto& counter = _failures_total.Add({
        {PA_METRIC_KEY_MESSAGE, msg}
    });
    counter.Increment();
    std::cout << msg << std::endl;
}

void ClientCollector::Metrics::observe_bytes_sent(std::size_t bytes_sent_) {
    bytes_sent.Observe(bytes_sent_);
}

void ClientCollector::Metrics::observe_bytes_recv(std::size_t bytes_recv_) {
    bytes_recv.Observe(bytes_recv_);
}
