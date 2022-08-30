/**
 * @copyright Copyright (c) 2021 ApertureData Inc.
 */

#include <thread>

#include "gtest/gtest.h"

#include "metrics/TimedQueue.h"
#include <prometheus/summary.h>

using namespace prometheus;
using namespace metrics;

TEST(TimedQueueTest, Histogram)
{
    Histogram::BucketBoundaries buckets{0.01};
    Histogram timer(buckets);
    Counter pushes;
    TimedQueue< int > queue(&timer, &pushes);
    EXPECT_EQ(0.0, pushes.Value());

    queue.push_back(1);
    EXPECT_EQ(1.0, pushes.Value());
    EXPECT_EQ(0, timer.Collect().histogram.sample_count);

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    queue.push_back(2);
    EXPECT_EQ(1, queue.release_front());
    EXPECT_EQ(2, queue.release_front());

    EXPECT_EQ(2.0, pushes.Value());
    auto timer_val = timer.Collect();
    EXPECT_EQ(2, timer_val.histogram.sample_count);

    EXPECT_EQ(2, timer_val.histogram.bucket.size());
    // total time spent in queue should exceed 10 ms
    EXPECT_LT(0.01, timer_val.histogram.sample_sum);
    // only one element should have spent less than 10 ms in the queue
    EXPECT_EQ(1, timer_val.histogram.bucket[0].cumulative_count);
}

TEST(TimedQueueTest, Summary)
{
    Summary::Quantiles qtiles{};
    Summary timer(qtiles);
    Counter pushes;
    TimedQueue< int, Summary > queue(&timer, &pushes);
    EXPECT_EQ(0.0, pushes.Value());

    queue.push_back(1);
    queue.push_back(2);
    EXPECT_EQ(2.0, pushes.Value());
    EXPECT_EQ(0, timer.Collect().summary.sample_count);
    EXPECT_EQ(1, queue.release_front());
    EXPECT_EQ(2, queue.release_front());

    EXPECT_EQ(2.0, pushes.Value());
    auto timer_val = timer.Collect();
    EXPECT_EQ(2, timer_val.summary.sample_count);
}

TEST(TimedQueueTest, Milliseconds)
{
    Histogram::BucketBoundaries buckets{10.0};
    Histogram ms_timer(buckets);
    Counter pushes;
    TimedQueue< int, Histogram, std::chrono::milliseconds > queue(&ms_timer, &pushes);
    EXPECT_EQ(0.0, pushes.Value());

    queue.push_back(1);
    EXPECT_EQ(1.0, pushes.Value());
    EXPECT_EQ(0, ms_timer.Collect().histogram.sample_count);

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    queue.push_back(2);
    EXPECT_EQ(1, queue.release_front());
    EXPECT_EQ(2, queue.release_front());

    EXPECT_EQ(2.0, pushes.Value());
    auto ms_timer_val = ms_timer.Collect();
    EXPECT_EQ(2, ms_timer_val.histogram.sample_count);

    EXPECT_EQ(2, ms_timer_val.histogram.bucket.size());
    // total time spent in queue should exceed 10 ms
    EXPECT_LT(10.0, ms_timer_val.histogram.sample_sum);
    // only one element should have spent less than 10 ms in the queue
    EXPECT_EQ(1, ms_timer_val.histogram.bucket[0].cumulative_count);
}
