/**
 * @copyright Copyright (c) 2021 ApertureData Inc.
 */

#pragma once

#include "comm/Enum.h"

namespace comm {

    ENUM_FLAGS(Protocol, uint8_t)
    {
        None = 0,
        TCP = 1 << 0,
        TLS = 1 << 1
    };

}
