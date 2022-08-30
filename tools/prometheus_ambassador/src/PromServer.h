/**
 *
 * @copyright Copyright (c) 2022 ApertureData Inc.
 *
 */

#pragma once

#include <atomic>
#include <memory>
#include "ClientCollector.h"
#include "PromConfig.h"
#include <prometheus/exposer.h>
#include <prometheus/registry.h>

class PromServer
{
    std::shared_ptr< prometheus::Registry > self_collector;
    std::shared_ptr< ClientCollector > client_collector;
    prometheus::Exposer exposer;

    // Handle ^c
    static std::atomic< bool > shutdown;
    static void sighandler(int signo);

   public:
    PromServer(const PromConfig& config);

    void run();
};
