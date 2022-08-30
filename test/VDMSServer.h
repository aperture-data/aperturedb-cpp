/**
 * @copyright Copyright (c) 2021 ApertureData Inc.
 */

#pragma once

#include <atomic>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "comm/ConnServer.h"

namespace VDMS
{

namespace protobufs
{

class queryMessage;

}

class VDMSServer
{
    std::unique_ptr< comm::ConnServer > _server;

   public:
    VDMSServer(int port, comm::ConnServerConfig config = {});
    ~VDMSServer();

   private:
    protobufs::queryMessage receive_message(const std::shared_ptr< comm::Connection >& connection);
    void send_message(const std::shared_ptr< comm::Connection >& connection,
                      const protobufs::queryMessage& protobuf_response);

    std::atomic< bool > _stop_signal{false};
    std::unique_ptr< std::thread > _work_thread{};
};
};  // namespace VDMS
