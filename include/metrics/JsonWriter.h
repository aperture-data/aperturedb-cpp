/**
 *
 * @copyright Copyright (c) 2022 ApertureData Inc.
 *
 */

#include "metrics/metric_schema_defines.h"
#include <prometheus/metric_family.h>
#include <sstream>
#include <limits>

namespace metrics
{

template< typename JSON >
class JsonWriterBase
{
    static constexpr int _prc{std::numeric_limits<double>::max_digits10 - 1};
public:

    using json_type = JSON;

    json_type to_json(int64_t val) {
        return json_type(val);
    }

    json_type to_json(uint64_t val) {
        return json_type(val);
    }

    json_type to_json(double val) {
        // We need to cast doubles to string because metrics make use of
        // +/-infinity and NaN which JSON doesn't support.
        std::ostringstream oss;
        oss.imbue(std::locale::classic());
        oss.precision(_prc);
        oss << val;
        return json_type(oss.str());
    }

    DISABLE_WARNING(return-type)
    json_type to_json(prometheus::MetricType type) {
        switch(type) {
        case prometheus::MetricType::Counter: return json_type("Counter");
        case prometheus::MetricType::Gauge: return json_type("Gauge");
        case prometheus::MetricType::Summary: return json_type("Summary");
        case prometheus::MetricType::Untyped: return json_type("Untyped");
        case prometheus::MetricType::Histogram: return json_type("Histogram");
        };
    }
    ENABLE_WARNING(return-type)

    json_type to_json(const std::string& val) {
        return json_type(val);
    }

    json_type to_json(const prometheus::ClientMetric::Bucket& bucket) {
        json_type val;
        val[AD_METRIC_SCHEMA_MAX] = to_json(bucket.upper_bound);
        val[AD_METRIC_SCHEMA_COUNT] = to_json(bucket.cumulative_count);
        return val;
    }

    json_type to_json(const prometheus::ClientMetric::Quantile& quantile) {
        json_type val;
        val[AD_METRIC_SCHEMA_QUANTILE] = to_json(quantile.quantile);
        val[AD_METRIC_SCHEMA_VALUE] = to_json(quantile.value);
        return val;
    }

    json_type to_json(const prometheus::ClientMetric& metric,
        prometheus::MetricType type)
    {
        json_type val;

        if (!metric.label.empty())
        {
            auto& labels = val[AD_METRIC_SCHEMA_LABELS];
            for (const auto& lbl : metric.label) {
                labels[lbl.name] = to_json(lbl.value);
            }
        }

        if (metric.timestamp_ms) {
            val[AD_METRIC_SCHEMA_TIMESTAMP_MS] = to_json(metric.timestamp_ms);
        }

        switch(type) {
        case prometheus::MetricType::Counter: {
            val[AD_METRIC_SCHEMA_VALUE] = to_json(metric.counter.value);
        } break;
        case prometheus::MetricType::Gauge: {
            val[AD_METRIC_SCHEMA_VALUE] = to_json(metric.gauge.value);
        } break;
        case prometheus::MetricType::Untyped: {
            val[AD_METRIC_SCHEMA_VALUE] = to_json(metric.untyped.value);
        } break;
        case prometheus::MetricType::Histogram:  {
            val[AD_METRIC_SCHEMA_COUNT] = to_json(metric.histogram.sample_count);
            val[AD_METRIC_SCHEMA_SUM] = to_json(metric.histogram.sample_sum);
            auto& buckets = val[AD_METRIC_SCHEMA_BUCKETS];
            for(const auto& bucket : metric.histogram.bucket) {
                buckets[buckets.size()] = to_json(bucket);
            }
        } break;
        case prometheus::MetricType::Summary:  {
            val[AD_METRIC_SCHEMA_COUNT] = to_json(metric.summary.sample_count);
            val[AD_METRIC_SCHEMA_SUM] = to_json(metric.summary.sample_sum);
            auto& quantiles = val[AD_METRIC_SCHEMA_QUANTILES];
            for(const auto& quantile : metric.summary.quantile) {
                quantiles[quantiles.size()] = to_json(quantile);
            }
        } break;
        };

        return val;
    }

    json_type to_json(const prometheus::MetricFamily& family) {
        json_type val;
        val[AD_METRIC_SCHEMA_NAME] = to_json(family.name);
        val[AD_METRIC_SCHEMA_HELP] = to_json(family.help);
        val[AD_METRIC_SCHEMA_TYPE] = to_json(family.type);
        auto& metrics = val[AD_METRIC_SCHEMA_METRICS];
        for(const auto& metric : family.metric) {
            metrics[metrics.size()] = to_json(metric, family.type);
        }
        return val;
    }

    json_type to_json(const std::vector<prometheus::MetricFamily>& metrics)
    {
        json_type val;
        auto& families = val[AD_METRIC_SCHEMA_FAMILIES];
        for(const auto family : metrics) {
            families[families.size()] = to_json(family);
        }
        return val;
    }
};

} // namespace metrics
