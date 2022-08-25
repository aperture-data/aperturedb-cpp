/**
 *
 * @copyright Copyright (c) 2021 ApertureData
 *
 */

#pragma once

#include <memory>
#include <prometheus/histogram.h>
#include <prometheus/counter.h>
#include "util/ScopeTimer.h"

namespace metrics
{

// A scoped timer that automatically records the duration
// of its lifetime to the provided metric.
template<
    typename METRIC_TYPE = prometheus::Histogram, // underlying metric type (histogram or summary)
    typename TIME_UNIT = std::chrono::seconds > // unit expected by the timer metric
class Timer
{
public:
    using this_type = Timer< METRIC_TYPE, TIME_UNIT >;
    using metric_type = METRIC_TYPE;
    using timer_type = ScopeTimer< TIME_UNIT, double >;
    using duration_type = typename timer_type::duration_type;

private:
    std::unique_ptr< timer_type > _timer;

public:
    explicit Timer(metric_type* metric = nullptr,
        prometheus::Counter* start_counter = nullptr)
    : _timer()
    {
        reset(metric, start_counter);
    }

    // not copyable
    explicit Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;

    // noexcept moveable
    explicit Timer(Timer&& other) noexcept
    : _timer(std::move(other._timer))
    {
    }

    Timer& operator=(Timer&& other) noexcept {
        if (&other != this) {
            _timer = std::move( other._timer );
        }
        return *this;
    }

    void reset(metric_type* metric = nullptr,
        prometheus::Counter* start_counter = nullptr)
    {
        if (metric) {
            _timer = std::make_unique< timer_type >([metric](double elapsed) {
                metric->Observe(elapsed);
            });
        }
        else {
            _timer.reset();
        }
        if ( start_counter ) start_counter->Increment();
    }
};
} // namespace metrics
