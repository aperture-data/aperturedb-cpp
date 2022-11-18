/**
 * @copyright Copyright (c) 2021 ApertureData Inc.
 */

#pragma once

#include <condition_variable>
#include <mutex>

// This is a generic barrier to sync initialization
// for client and server in test
// This barrier will block until "count" number
// of threads reach the barrier point.
class Barrier
{
   public:
    explicit Barrier(std::size_t count);

    void wait();
    void reset();

   private:
    std::condition_variable _condition_variable{};
    std::size_t _count;
    std::size_t _initial_count;
    std::mutex _mutex{};
};
