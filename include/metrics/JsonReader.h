/**
 *
 * @copyright Copyright (c) 2022 ApertureData Inc.
 *
 */

#include "metrics/metric_schema_defines.h"
#include <prometheus/metric_family.h>

namespace metrics
{
template< typename T >
struct ParseAs
{
    template< typename JSON >
    T operator()(const JSON& json) {
        return json.template get<T>();
    }
};

template<>
struct ParseAs<double>
{
    template< typename JSON >
    double operator()(const JSON& json) {
        // we need to handle doubles serialized as strings
        if (json.is_string())
            return std::stod(json.template get_ref<const std::string&>());
        return json.template get<double>();
    }
};

template<>
struct ParseAs<prometheus::MetricType>
{
    template< typename JSON >
    prometheus::MetricType operator()(const JSON& json) {
        if (json.is_string()) {
            switch(json.template get_ref<const std::string&>()[0]) {
                case 'C': return prometheus::MetricType::Counter;
                case 'G': return prometheus::MetricType::Gauge;
                case 'S': return prometheus::MetricType::Summary;
                case 'U': return prometheus::MetricType::Untyped;
                case 'H': return prometheus::MetricType::Histogram;
                default: break;
            };
        }
        return prometheus::MetricType::Untyped;
    }
};

template< typename JSON >
class JsonReader {
public:
    using json_type = JSON;

private:

    template<typename T>
    void assign_from(T& val, const json_type& json) {
        val = ParseAs<T>()(json);
    }

    prometheus::ClientMetric& emplace_metric_metadata(prometheus::MetricFamily& fam, const json_type& mtc) {
        fam.metric.emplace_back();
        auto& metric = fam.metric.back();

        auto mtc_labels = mtc.find(AD_METRIC_SCHEMA_LABELS);
        if (mtc_labels != mtc.end()) {
            metric.label.reserve(mtc_labels->size());
            for (auto lbl : mtc_labels->items()) {
                metric.label.emplace_back();
                metric.label.back().name = lbl.key();
                assign_from(metric.label.back().value, lbl.value());
            }
        }

        auto mtc_ts = mtc.find(AD_METRIC_SCHEMA_TIMESTAMP_MS);
        if (mtc_ts != mtc.end()) {
            assign_from(metric.timestamp_ms, mtc_ts.value());
        }

        return metric;
    }

    void parse_counter_values(prometheus::MetricFamily& fam, const json_type& json) {
        for (const auto& mtc : json) {
            auto& counter = emplace_metric_metadata(fam, mtc).counter;
            assign_from(counter.value, mtc[AD_METRIC_SCHEMA_VALUE]);
        }
    }

    void parse_gauge_values(prometheus::MetricFamily& fam, const json_type& json) {
        for (const auto& mtc : json) {
            auto& gauge = emplace_metric_metadata(fam, mtc).gauge;
            assign_from(gauge.value, mtc[AD_METRIC_SCHEMA_VALUE]);
        }
    }

    void parse_summary_values(prometheus::MetricFamily& fam, const json_type& json) {
        for (const auto& mtc : json) {
            auto& summary = emplace_metric_metadata(fam, mtc).summary;
            assign_from(summary.sample_count, mtc[AD_METRIC_SCHEMA_COUNT]);
            assign_from(summary.sample_sum, mtc[AD_METRIC_SCHEMA_SUM]);
            const auto& mtc_qnt = mtc[AD_METRIC_SCHEMA_QUANTILES];
            summary.quantile.reserve(mtc_qnt.size());
            for (const auto& qnt : mtc_qnt) {
                summary.quantile.emplace_back();
                assign_from(summary.quantile.back().quantile, qnt[AD_METRIC_SCHEMA_QUANTILE]);
                assign_from(summary.quantile.back().value, qnt[AD_METRIC_SCHEMA_VALUE]);
            }
        }
    }

    void parse_untyped_values(prometheus::MetricFamily& fam, const json_type& json) {
        for (const auto& mtc : json) {
            auto& untyped = emplace_metric_metadata(fam, mtc).untyped;
            assign_from(untyped.value, mtc[AD_METRIC_SCHEMA_VALUE]);
        }
    }

    void parse_histogram_values(prometheus::MetricFamily& fam, const json_type& json) {
        for (const auto& mtc : json) {
            auto& histogram = emplace_metric_metadata(fam, mtc).histogram;
            assign_from(histogram.sample_count, mtc[AD_METRIC_SCHEMA_COUNT]);
            assign_from(histogram.sample_sum, mtc[AD_METRIC_SCHEMA_SUM]);
            const auto& mtc_buc = mtc[AD_METRIC_SCHEMA_BUCKETS];
            histogram.bucket.reserve(mtc_buc.size());
            for (const auto& buc : mtc_buc) {
                histogram.bucket.emplace_back();
                assign_from(histogram.bucket.back().cumulative_count, buc[AD_METRIC_SCHEMA_COUNT]);
                assign_from(histogram.bucket.back().upper_bound, buc[AD_METRIC_SCHEMA_MAX]);
            }
        }
    }

    void parse_family_values(prometheus::MetricFamily& fam, const json_type& json) {
        switch(fam.type) {
        case prometheus::MetricType::Counter: return parse_counter_values(fam, json);
        case prometheus::MetricType::Gauge: return parse_gauge_values(fam, json);
        case prometheus::MetricType::Summary: return parse_summary_values(fam, json);
        case prometheus::MetricType::Untyped: return parse_untyped_values(fam, json);
        case prometheus::MetricType::Histogram: return parse_histogram_values(fam, json);
        }
    }

public:
    std::vector<prometheus::MetricFamily> parse_metrics(const json_type& json) {
        std::vector<prometheus::MetricFamily> result;
        for (const auto& fam_json : json[AD_METRIC_SCHEMA_FAMILIES]) {
            result.emplace_back();
            assign_from(result.back().name, fam_json[AD_METRIC_SCHEMA_NAME]);
            assign_from(result.back().help, fam_json[AD_METRIC_SCHEMA_HELP]);
            assign_from(result.back().type, fam_json[AD_METRIC_SCHEMA_TYPE]);
            parse_family_values(result.back(), fam_json[AD_METRIC_SCHEMA_METRICS]);
        }
        return result;
    }
};
} // namespace metrics
