/**
 * @copyright Copyright (c) 2021 ApertureData Inc.
 */

#pragma once

#include <condition_variable>
#include <mutex>

class Barrier
{
public:

    explicit Barrier(std::size_t count);

    void wait();

private:

    std::condition_variable _condition_variable{};
    std::size_t _count;
    std::mutex _mutex{};
};