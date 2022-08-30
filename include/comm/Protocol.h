/**
 * @copyright Copyright (c) 2021 ApertureData Inc.
 */

#pragma once

#include "comm/Enum.h"

namespace comm
{

// We are setting Protocol to be uint32_t here because
// otherwise there will be padding, and we
// observed that sometimes that padding is causing issues
// on the client side when decoding the protocol.
// TODO: Make HelloMessage and Protocol a Protobuf message.
// to prevent this kind of issues, and to prevent
// potential issues with endianness.
ENUM_FLAGS(Protocol, uint32_t){None = 0, TCP = 1 << 0, TLS = 1 << 1, Any = TCP | TLS};

}  // namespace comm
