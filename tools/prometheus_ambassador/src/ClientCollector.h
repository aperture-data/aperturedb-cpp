/**
 *
 * @copyright Copyright (c) 2022 ApertureData Inc.
 *
 */

#pragma once

#include <memory>
#include <aperturedb/VDMSClient.h>
#include <prometheus/registry.h>
#include <prometheus/histogram.h>
#include <prometheus/counter.h>
#include <prometheus/gauge.h>
#include "PromConfig.h"

class ClientCollector : public prometheus::Collectable
{
    const PromConfig& config;
    mutable std::unique_ptr<VDMS::VDMSClient> _client;

    class Metrics
    {
        prometheus::Labels _static_labels;
        prometheus::Family< prometheus::Gauge >& _client_connected;
        prometheus::Family< prometheus::Counter >& _failures_total;
        prometheus::Family< prometheus::Histogram >& _client_query_us;
        prometheus::Histogram::BucketBoundaries _client_query_buckets;

    public:
        prometheus::Gauge& client_connected;
        prometheus::Histogram& connect_timer;
        prometheus::Histogram& query_timer;
        prometheus::Histogram& parse_timer;

        void increment_failure(const std::string& msg);

        Metrics(const PromConfig& config, prometheus::Registry& registry);
    };
    mutable Metrics _metrics;

    void connect() const;

public:
    ClientCollector(const PromConfig& config, prometheus::Registry& registry);

    std::vector<prometheus::MetricFamily> Collect() const override;
};
