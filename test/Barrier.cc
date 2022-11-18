/**
 * @copyright Copyright (c) 2021 ApertureData Inc.
 */

#include "Barrier.h"

Barrier::Barrier(std::size_t count) : _count(count), _initial_count(count) {}

void Barrier::wait()
{
    std::unique_lock< std::mutex > lock(_mutex);

    if (--_count == 0) {
        _condition_variable.notify_all();
    } else {
        _condition_variable.wait(lock, [this] { return _count == 0; });
    }
}

void Barrier::reset() { _count = _initial_count; }
