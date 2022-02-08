/**
 * @copyright Copyright (c) 2021 ApertureData Inc.
 */

#include <thread>

#include "gtest/gtest.h"

#include "ClientCollector.h"
#include "prometheus_ambassador_defines.h"

#include "Barrier.h"
#include "comm/ConnServer.h"

#include "aperturedb/gcc_util.h" // DISABLE_WARNING
DISABLE_WARNING(effc++)
DISABLE_WARNING(useless-cast)
DISABLE_WARNING(suggest-override)
#include "aperturedb/queryMessage.pb.h"
ENABLE_WARNING(suggest-override)
ENABLE_WARNING(useless-cast)
ENABLE_WARNING(effc++)

#define SERVER_PORT_INTERCHANGE 43210

TEST(ClientCollectorTest, CollectClientMetrics)
{

    Barrier barrier(2);

    std::thread server_thread([&]()
    {
        std::string expected_query = R"(
"[{"GetMetrics":{"format":"json"}}])";

        auto response = R"([{
            "GetMetrics": {
                "status": 0,
                "version": "0.9.2",
                "values": {
                    "families": [{
                        "name": "cpu_cycles_total",
                        "help": "Elapsed CPU cycles",
                        "type": "Counter",
                        "metrics": [
                            {"labels": {"type": "system"}, "value": "2828933849"},
                            {"labels": {"type": "application"}, "value": "27"}
                        ]
                    },{
                        "name": "memory_bytes",
                        "help": "Bytes of memory in use",
                        "type": "Gauge",
                        "metrics": [
                            {"labels": {"type": "total"}, "value": "33675821056"},
                            {"labels": {"type": "virtual"}, "value": "3303710560256"},
                            {"labels": {"type": "physical"}, "value": "57339904"}
                        ]
                    },
                    {
                        "name": "wait_seconds_total",
                        "help": "Amount of time spent waiting in the work queue, in seconds",
                        "type": "Histogram",
                        "metrics": [{
                            "labels": {"query_type": "retry"},
                            "count": 0,
                            "sum": "0",
                            "buckets": [
                                {"count": 0, "max": "0.001"},
                                {"count": 0, "max": "0.01"},
                                {"count": 0, "max": "0.1"},
                                {"count": 0, "max": "1"},
                                {"count": 0, "max": "10"},
                                {"count": 0, "max": "60"},
                                {"count": 0, "max": "inf"}
                            ]
                        },{
                            "labels": {"query_type": "read_write"},
                            "count": 1,
                            "sum": "1.7489e-05",
                            "buckets": [
                                {"count": 1, "max": "0.001"},
                                {"count": 1, "max": "0.01"},
                                {"count": 1, "max": "0.1"},
                                {"count": 1, "max": "1"},
                                {"count": 1, "max": "10"},
                                {"count": 1, "max": "60"},
                                {"count": 1, "max": "inf"}
                            ]
                        },{
                            "labels": {"query_type": "read_only"},
                            "count": 0,
                            "sum": "0",
                            "buckets": [
                                {"count": 0, "max": "0.001"},
                                {"count": 0, "max": "0.01"},
                                {"count": 0, "max": "0.1"},
                                {"count": 0, "max": "1"},
                                {"count": 0, "max": "10"},
                                {"count": 0, "max": "60"},
                                {"count": 0, "max": "inf"}
                            ]
                        }]
                    }]
                }
            }
        }])"_json;

        comm::ConnServer server(SERVER_PORT_INTERCHANGE);

        barrier.wait();

        auto server_conn = server.accept();

        auto message_received = server_conn->recv_message();
        std::string recv_message(reinterpret_cast<const char*>(message_received.data()));
        EXPECT_EQ(recv_message, expected_query);

        VDMS::protobufs::queryMessage cmd;
        cmd.set_json(response.dump());
        std::basic_string<uint8_t> msg(cmd.ByteSizeLong(), 0);
        cmd.SerializeToArray(const_cast<uint8_t*>(msg.data()), msg.length());
        server_conn->send_message(msg.data(), msg.length());
    });

    nlohmann::json cfg_json;
    cfg_json[PA_CONFIG_VDMS_PORT_KEY] = SERVER_PORT_INTERCHANGE;
    PromConfig cfg(cfg_json);
    prometheus::Registry reg;
    ClientCollector cc(cfg, reg);

    barrier.wait();

    auto metrics = cc.Collect();

    server_thread.join();

    ASSERT_GT(metrics.size(), 0);
}

TEST(ClientCollectorTest, UnableToConnect)
{
    nlohmann::json cfg_json;
    cfg_json[PA_CONFIG_VDMS_PORT_KEY] = SERVER_PORT_INTERCHANGE;
    PromConfig cfg(cfg_json);
    prometheus::Registry reg;
    ClientCollector cc(cfg, reg);

    auto metrics = cc.Collect();
    auto self_metrics = reg.Collect();

    EXPECT_EQ(metrics.size(), 0);
    EXPECT_EQ(self_metrics.size(), 3);
}
