/**
 *
 * @copyright Copyright (c) 2022 ApertureData Inc.
 *
 */

#include <cstring>
#include <thread>
#include <iostream>
#include <aperturedb/Exception.h>
#include <comm/Exception.h>
#include "ClientCollector.h"
#include <prometheus/metric_family.h>
#include <time.h>

#include <cxxabi.h>
#include <sstream>

namespace {
    template< typename T >
    inline std::string type_name(const T& t) {
        return abi::__cxa_demangle(typeid((t)).name(), nullptr, nullptr, nullptr);
    }

    template<typename EX>
    std::string print_aperture_exception(const EX& e) {
        std::ostringstream oss;
        oss << type_name(e) << " " << e.name << " (" << e.file << ":" << e.line << ")";
        if (e.errno_val != 0)
            oss << " [code=" << strerror(e.errno_val) << "]";
        if (!e.msg.empty())
            oss << " " << e.msg;
        return oss.str();
    }

    // must be called within a catch{} block
    std::string print_caught_exception() {
        try {
            throw;
        }
        catch (const VDMS::Exception& e) {
            return print_aperture_exception(e);
        }
        catch (const comm::Exception& e) {
            return print_aperture_exception(e);
        }
        catch (const std::exception& e) {
            return std::string("[") + type_name(e) + "] " + e.what();
        }
        catch (...) {
            return "<unknown exception type>";
        }
    }

    inline double diff_in_seconds(const timespec& start, const timespec& end) {
        return ((end.tv_nsec - start.tv_nsec) * 1e-9) + (end.tv_sec - start.tv_sec);
    }
}

void ClientCollector::connect() const {
    if (!config.username.empty() && !config.password.empty()) {
        _client.reset(new VDMS::VDMSClient(
            config.username,
            config.password,
            config.vdms_address,
            config.vdms_port,
            config.protocols,
            config.ca_certificate));
    }
    else if (!config.api_token.empty()) {
        _client.reset(new VDMS::VDMSClient(
            config.api_token,
            config.vdms_address,
            config.vdms_port,
            config.protocols,
            config.ca_certificate));
    }
    else {
        _client.reset(new VDMS::VDMSClient(
            config.vdms_address,
            config.vdms_port,
            config.protocols,
            config.ca_certificate));
    }
    _metrics.connections_total.Increment();
    std::cout << "Prometheus ambassador connected to " << config.vdms_address << ":" << config.vdms_port << std::endl;
}

ClientCollector::ClientCollector(const PromConfig& cfg, prometheus::Registry& registry)
: config(cfg)
, _client(nullptr)
, _metrics(config, registry)
{
}

namespace
{
    template< typename T >
    T parse_as(const nlohmann::json& json) {
        return json.get<T>();
    }

    template<>
    double parse_as<double>(const nlohmann::json& json) {
        // we need to handle doubles serialized as strings
        if (json.is_string())
            return std::stod(json.get_ref<const std::string&>());
        return json.get<double>();
    }

    template<>
    prometheus::MetricType parse_as<prometheus::MetricType>(const nlohmann::json& json) {
        if (json.is_string()) {
            switch(json.get_ref<const std::string&>()[0]) {
                case 'C': return prometheus::MetricType::Counter;
                case 'G': return prometheus::MetricType::Gauge;
                case 'S': return prometheus::MetricType::Summary;
                case 'U': return prometheus::MetricType::Untyped;
                case 'H': return prometheus::MetricType::Histogram;
                default: break;
            };
        }
        std::cout << "Invalid metric type value: " << json.dump() << std::endl;
        return prometheus::MetricType::Untyped;
    }

    template<typename T>
    void assign_from(T& val, const nlohmann::json& json) {
        val = parse_as<T>(json);
    }

    prometheus::ClientMetric& emplace_metric_metadata(prometheus::MetricFamily& fam, const nlohmann::json& mtc) {
        fam.metric.emplace_back();
        auto& metric = fam.metric.back();

        auto mtc_labels = mtc.find("labels");
        if (mtc_labels != mtc.end()) {
            metric.label.reserve(mtc_labels->size());
            for (auto lbl : mtc_labels->items()) {
                metric.label.emplace_back();
                metric.label.back().name = lbl.key();
                assign_from(metric.label.back().value, lbl.value());
            }
        }

        auto mtc_ts = mtc.find("timestamp_ms");
        if (mtc_ts != mtc.end()) {
            assign_from(metric.timestamp_ms, mtc_ts.value());
        }

        return metric;
    }

    void parse_counter_values(prometheus::MetricFamily& fam, const nlohmann::json& json) {
        for (const auto& mtc : json) {
            auto& counter = emplace_metric_metadata(fam, mtc).counter;
            assign_from(counter.value, mtc["value"]);
        }
    }

    void parse_gauge_values(prometheus::MetricFamily& fam, const nlohmann::json& json) {
        for (const auto& mtc : json) {
            auto& gauge = emplace_metric_metadata(fam, mtc).gauge;
            assign_from(gauge.value, mtc["value"]);
        }
    }

    void parse_summary_values(prometheus::MetricFamily& fam, const nlohmann::json& json) {
        for (const auto& mtc : json) {
            auto& summary = emplace_metric_metadata(fam, mtc).summary;
            assign_from(summary.sample_count, mtc["count"]);
            assign_from(summary.sample_sum, mtc["sum"]);
            const auto& mtc_qnt = mtc["quantiles"];
            summary.quantile.reserve(mtc_qnt.size());
            for (const auto& qnt : mtc_qnt) {
                summary.quantile.emplace_back();
                assign_from(summary.quantile.back().quantile, qnt["quantile"]);
                assign_from(summary.quantile.back().value, qnt["value"]);
            }
        }
    }

    void parse_untyped_values(prometheus::MetricFamily& fam, const nlohmann::json& json) {
        for (const auto& mtc : json) {
            auto& untyped = emplace_metric_metadata(fam, mtc).untyped;
            assign_from(untyped.value, mtc["value"]);
        }
    }

    void parse_histogram_values(prometheus::MetricFamily& fam, const nlohmann::json& json) {
        for (const auto& mtc : json) {
            auto& histogram = emplace_metric_metadata(fam, mtc).histogram;
            assign_from(histogram.sample_count, mtc["count"]);
            assign_from(histogram.sample_sum, mtc["sum"]);
            const auto& mtc_buc = mtc["buckets"];
            histogram.bucket.reserve(mtc_buc.size());
            for (const auto& buc : mtc_buc) {
                histogram.bucket.emplace_back();
                assign_from(histogram.bucket.back().cumulative_count, buc["count"]);
                assign_from(histogram.bucket.back().upper_bound, buc["max"]);
            }
        }
    }

    void parse_family_values(prometheus::MetricFamily& fam, const nlohmann::json& json) {
        switch(fam.type) {
        case prometheus::MetricType::Counter: return parse_counter_values(fam, json);
        case prometheus::MetricType::Gauge: return parse_gauge_values(fam, json);
        case prometheus::MetricType::Summary: return parse_summary_values(fam, json);
        case prometheus::MetricType::Untyped: return parse_untyped_values(fam, json);
        case prometheus::MetricType::Histogram: return parse_histogram_values(fam, json);
        }
    }

    std::vector<prometheus::MetricFamily> parse_metrics(const nlohmann::json& json) {
        std::vector<prometheus::MetricFamily> result;
        for (const auto& fam_json : json["values"]["families"]) {
            result.emplace_back();
            assign_from(result.back().name, fam_json["name"]);
            assign_from(result.back().help, fam_json["help"]);
            assign_from(result.back().type, fam_json["type"]);
            parse_family_values(result.back(), fam_json["metrics"]);
        }
        return result;
    }
} // namespace

std::vector<prometheus::MetricFamily> ClientCollector::Collect() const {
    auto query = R"(
        [{"GetMetrics": {
            "format": "json"
        }}]
    )"_json;

    try {
        timespec start;
        clock_gettime(CLOCK_MONOTONIC, &start);

        if (!_client)
            connect();

        auto res = _client->query(query.dump());

        auto response = nlohmann::json::parse(res.json);
        if (response.is_array()) {
            auto out = parse_metrics(response[0]["GetMetrics"]);

            timespec end;
            clock_gettime(CLOCK_MONOTONIC, &end);
            _metrics.query_seconds.Observe(diff_in_seconds(start, end));
            _metrics.queries_total.Increment();
            return out;
        }
        std::cout << "Unexpected response: " << response.dump() << std::endl;
    }
    catch (...) {
        auto msg = print_caught_exception();
        _metrics.increment_failure(!!_client, msg);
        std::cout << msg << std::endl;
    }

    // failed
    if (_client) {
        std::cout << "Client connection closed" << std::endl;
        _client.reset();
    }
    return {};
}

ClientCollector::Metrics::Metrics(const PromConfig& config, prometheus::Registry& registry)
: _static_labels({
    {"host", config.vdms_address},
    {"port", std::to_string(config.vdms_port)}
})
, _client_connections_total(prometheus::BuildCounter()
    .Name("client_connections")
    .Help("Times the prometheus adaptor attempted to connect to the ApertureDB API")
    .Labels(_static_labels)
    .Register(registry))
, _client_queries_total(prometheus::BuildCounter()
    .Name("client_queries")
    .Help("Times the prometheus adaptor queried ApertureDB metrics")
    .Labels(_static_labels)
    .Register(registry))
, _client_query_seconds(prometheus::BuildHistogram()
    .Name("client_query_seconds")
    .Help("Time taken by the prometheus adaptor to query metrics from ApertureDB, in seconds")
    .Labels(_static_labels)
    .Register(registry))
, _client_query_buckets({0.001, 0.01, 0.1, 1.0, 10.0})
, connections_total(_client_connections_total.Add({{"result", "success"}}))
, queries_total(_client_queries_total.Add({{"result", "success"}}))
, query_seconds(_client_query_seconds.Add({}, _client_query_buckets))
{
}

void ClientCollector::Metrics::increment_failure(bool has_connection, const std::string& msg) {
    auto& fam = (has_connection ? _client_queries_total : _client_connections_total );
    auto& counter = fam.Add({
        {"result", "failure"},
        {"details", msg}
    });
    counter.Increment();
}
