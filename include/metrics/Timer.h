/**
 *
 * @copyright Copyright (c) 2021 ApertureData
 *
 */

#pragma once

#include <time.h>
#include <math.h>
#include <chrono>
#include <prometheus/histogram.h>
#include <prometheus/counter.h>

namespace metrics
{

// A scoped timer that automatically records the duration
// of its lifetime to the provided metric.
template<
    typename TIME_UNIT = std::chrono::seconds, // unit expected by the timer metric
    typename METRIC_TYPE = prometheus::Histogram > // underlying metric type (histogram or summary)
class Timer
{
public:
    using duration_type = typename std::chrono::duration< double, typename TIME_UNIT::period >;
    using metric_type = METRIC_TYPE;

private:
    metric_type* _timer;
    timespec _start;

public:
    explicit Timer(metric_type* timer = nullptr,
        prometheus::Counter* start_counter = nullptr)
    : _timer(timer)
    , _start()
    {
        clock_gettime(CLOCK_MONOTONIC, &_start);
        if (start_counter)
            start_counter->Increment();
    }

    ~Timer() {
        try {
            reset();
        } catch (...) {}
    }

    // not copyable
    explicit Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;

    // noexcept moveable
    explicit Timer(Timer&& other) noexcept
    : _timer( other._timer )
    , _start( std::move(other._start) )
    {
        other._timer = nullptr;
    }

    Timer& operator=(Timer&& other) noexcept {
        _timer = other._timer;
        other._timer = nullptr;
        _start = std::move( other._start );
        return *this;
    }

    void reset(metric_type* timer = nullptr,
        prometheus::Counter* start_counter = nullptr)
    {
        timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        if (_timer) {
            duration_type dur =
                std::chrono::nanoseconds(now.tv_nsec - _start.tv_nsec) +
                std::chrono::seconds(now.tv_sec - _start.tv_sec);
            _timer->Observe(dur.count());
        }
        _timer = timer;
        _start = std::move( now );
        if ( start_counter ) start_counter->Increment();
    }
};
} // namespace metrics
