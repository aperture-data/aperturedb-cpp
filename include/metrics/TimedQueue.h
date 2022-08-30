/**
 *
 * @copyright Copyright (c) 2021 ApertureData
 *
 */

#pragma once

#include <list>
#include "metrics/Timer.h"

namespace metrics
{

// This is a one-way queue container that automatically observes how much
// time each element spends inside.
template < typename T,  // queue value type
           typename METRIC_TYPE =
               prometheus::Histogram,  // underlying metric type (histogram or summary)
           typename TIME_UNIT = std::chrono::seconds >  // unit expected by the underlying metric
class TimedQueue
{
   public:
    using value_type  = T;
    using metric_type = METRIC_TYPE;
    using timer_type  = Timer< metric_type, TIME_UNIT >;

   private:
    std::list< std::pair< value_type, timer_type > > _queue;
    metric_type* _wait_timer;
    prometheus::Counter* _push_counter;

   public:
    explicit TimedQueue(metric_type* timer = nullptr, prometheus::Counter* push_counter = nullptr)
        : _queue(), _wait_timer(timer), _push_counter(push_counter)
    {
    }

    // copyable
    explicit TimedQueue(const TimedQueue&) = default;
    TimedQueue& operator=(const TimedQueue&) = default;

    // moveable
    explicit TimedQueue(TimedQueue&&) = default;
    TimedQueue& operator=(TimedQueue&&) = default;

    void push_back(const value_type& val)
    {
        _queue.emplace_back(std::piecewise_construct_t(),
                            std::forward_as_tuple(val),
                            std::forward_as_tuple(_wait_timer, _push_counter));
    }

    void push_back(value_type&& val)
    {
        _queue.emplace_back(std::piecewise_construct_t(),
                            std::forward_as_tuple(std::move(val)),
                            std::forward_as_tuple(_wait_timer, _push_counter));
    }

    template < typename... Args >
    void emplace_back(Args&&... args)
    {
        _queue.emplace_back(std::piecewise_construct_t(),
                            std::forward_as_tuple(std::forward< Args >(args)...),
                            std::forward_as_tuple(_wait_timer, _push_counter));
    }

    void pop_front() { _queue.pop_front(); }
    bool empty() const { return _queue.empty(); }
    size_t size() const { return _queue.size(); }
    const value_type& front() const { return _queue.front().first; }

    value_type release_front()
    {
        value_type out = std::move(_queue.front().first);
        _queue.pop_front();
        return out;
    }
};

}  // namespace metrics
