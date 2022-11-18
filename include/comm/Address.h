/**
 * @copyright Copyright (c) 2021 ApertureData Inc.
 */

#pragma once

#include <string>

namespace comm
{

struct Address {
    std::string addr;
    int port;
    int unix_addr{0};
};

};  // namespace comm
