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
#include "PromConfig.h"

class ClientCollector : public prometheus::Collectable
{
    const PromConfig& config;
    mutable std::unique_ptr<VDMS::VDMSClient> _client;

    class Metrics
    {
        prometheus::Labels _static_labels;
        prometheus::Family< prometheus::Counter >& _client_connections_total;
        prometheus::Family< prometheus::Counter >& _client_queries_total;
        prometheus::Family< prometheus::Histogram >& _client_query_seconds;
        prometheus::Histogram::BucketBoundaries _client_query_buckets;

    public:
        prometheus::Counter& connections_total;
        prometheus::Counter& queries_total;
        prometheus::Histogram& query_seconds;

        void increment_failure(bool has_connection, const std::string& msg);

        Metrics(const PromConfig& config, prometheus::Registry& registry);
    };
    mutable Metrics _metrics;

    void connect() const;

public:
    ClientCollector(const PromConfig& config, prometheus::Registry& registry);

    std::vector<prometheus::MetricFamily> Collect() const override;
};
