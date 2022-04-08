/**
 *
 * @copyright Copyright (c) 2022 ApertureData Inc.
 *
 */

#pragma once

#include <memory>
#include <aperturedb/VDMSClient.h>
#include "util/gcc_util.h"
DISABLE_WARNING(effc++)
#include <prometheus/registry.h>
#include <prometheus/summary.h>
#include <prometheus/histogram.h>
#include <prometheus/counter.h>
#include <prometheus/gauge.h>
ENABLE_WARNING(effc++)
#include "PromConfig.h"
#include "comm/Connection.h"

class ClientCollector : public prometheus::Collectable
{
    const PromConfig& _config;
    mutable std::unique_ptr<VDMS::VDMSClient> _client;
    prometheus::Registry& _registry;

    class Metrics : public comm::ConnMetrics
    {
        prometheus::Labels _static_labels;
        prometheus::Family< prometheus::Gauge >& _client_connected;
        prometheus::Family< prometheus::Counter >& _failures_total;
        prometheus::Family< prometheus::Summary >& _client_query_sec;
        prometheus::Summary::Quantiles _client_query_quantiles;
        prometheus::Family< prometheus::Histogram >& _bytes_transferred;
        prometheus::Histogram::BucketBoundaries _bytes_transferred_buckets;

    public:
        prometheus::Gauge& client_connected;
        prometheus::Summary& connect_timer;
        prometheus::Summary& query_timer;
        prometheus::Summary& parse_timer;
        prometheus::Histogram& bytes_sent;
        prometheus::Histogram& bytes_recv;

        void increment_failure(const std::string& msg);

        void observe_bytes_sent(std::size_t bytes_sent) override;
        void observe_bytes_recv(std::size_t bytes_recv) override;

        Metrics(const PromConfig& config, prometheus::Registry& registry);
    };
    mutable Metrics _metrics;

    void connect() const;

public:
    ClientCollector(const PromConfig& config, prometheus::Registry& registry);

    std::vector<prometheus::MetricFamily> Collect() const override;
};
