/**
 * @copyright Copyright (c) 2021 ApertureData Inc.
 */

#pragma once

#include "comm/Protocol.h"

namespace comm {

    struct HelloMessage {
        uint32_t version{};
        Protocol protocol{};
    };

}
