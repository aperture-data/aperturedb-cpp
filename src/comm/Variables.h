/**
 * @copyright Copyright (c) 2021 ApertureData Inc.
 */

#pragma once

namespace comm
{

const unsigned PROTOCOL_VERSION = 1;

const unsigned MAX_PORT_NUMBER = 65535;

const unsigned MAX_CONN_QUEUE        = 2048;
const unsigned MAX_RECV_TIMEOUT_SECS = 600;  // 10 mins should be plenty

const unsigned MIN_BUFFER_SIZE     = 1024 * 1;            //   1KB
const unsigned MAX_BUFFER_SIZE     = 1024 * 1024 * 1024;  //   1GB
const unsigned DEFAULT_BUFFER_SIZE = 1024 * 1024 * 256;   // 256MB

};  // namespace comm
