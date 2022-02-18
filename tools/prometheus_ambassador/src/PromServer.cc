/**
 *
 * @copyright Copyright (c) 2022 ApertureData Inc.
 *
 */

#include <thread>
#include <cstring>
#include <csignal>
#include <iostream>

#include "PromServer.h"

std::atomic<bool> PromServer::shutdown{false};

PromServer::PromServer(const PromConfig& config)
: self_collector(std::make_shared<prometheus::Registry>())
, client_collector(std::make_shared<ClientCollector>(config, *self_collector))
, exposer(config.prometheus_address + ':' + std::to_string(config.prometheus_port), 1)
{
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = PromServer::sighandler;
    if (sigaction(SIGINT, &action, NULL) != 0)
        throw std::runtime_error("failed to install signal handler");
    if (sigaction(SIGTERM, &action, NULL) != 0)
        throw std::runtime_error("failed to install signal handler");
    if (sigaction(SIGQUIT, &action, NULL) != 0)
        throw std::runtime_error("failed to install signal handler");

    std::cout << "Prometheus ambassador listening on "
        << config.prometheus_address << ":" << config.prometheus_port
        << std::endl;
}

namespace
{
    class CollectableRegistration
    {
        prometheus::Exposer& exposer;
        std::weak_ptr<prometheus::Collectable> collector;
    public:
        CollectableRegistration(prometheus::Exposer& exp,
            const std::shared_ptr<prometheus::Collectable>& coll)
        : exposer(exp)
        , collector(coll)
        {
            exposer.RegisterCollectable(collector);
        }

        ~CollectableRegistration()
        {
            try {
                exposer.RemoveCollectable(collector);
            }
            catch (...) {}
        }
    };
}

void PromServer::run() {
    CollectableRegistration register_self(exposer, self_collector);
    CollectableRegistration register_client(exposer, client_collector);

    while(!shutdown) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void PromServer::sighandler(int signo)
{
    shutdown = (signo == SIGINT) || (signo == SIGTERM)|| (signo == SIGQUIT);
}
