/**
 *
 * @copyright Copyright (c) 2022 ApertureData
 *
 */

#pragma once

#include <time.h>
#include <math.h>
#include <chrono>

// A scoped timer that calls the provided callback on destruction passing the elapsed time as arg.
// Usage example:
// void profile_foo() {
//     ScopeTimer<> _timer([](double elapsed_sec){
//         std::cout << "foo() took " << elapsed_sec << " seconds." << std::endl;
//     });
//     foo();
// }
template<
    typename TIME_UNIT = std::chrono::seconds,
    typename TIME_REP = double
>
class ScopeTimer {
public:
    using callback_type = std::function< void(TIME_REP) >;
    using duration_type = typename std::chrono::duration< TIME_REP, typename TIME_UNIT::period >;

private:
    callback_type _cb;
    timespec _start;

public:
    explicit ScopeTimer(callback_type cb)
    : _cb(std::move(cb))
    , _start()
    {
        clock_gettime(CLOCK_MONOTONIC, &_start);
    }

    ~ScopeTimer() {
        try {
            timespec now;
            clock_gettime(CLOCK_MONOTONIC, &now);
            if (_cb) {
                duration_type dur =
                    std::chrono::nanoseconds(now.tv_nsec - _start.tv_nsec) +
                    std::chrono::seconds(now.tv_sec - _start.tv_sec);
                _cb(std::move(dur).count());
            }
        } catch (...) {}
    }
};
