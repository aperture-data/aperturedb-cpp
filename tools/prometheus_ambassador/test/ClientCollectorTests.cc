/**
 * @copyright Copyright (c) 2021 ApertureData Inc.
 */

#include <thread>

#include "gtest/gtest.h"
#include "ClientCollector.h"
#include "prometheus_ambassador_defines.h"
#include "Barrier.h"
#include "comm/ConnServer.h"
#include "aperturedb/queryMessageWrapper.h"
#include "metrics/JsonWriter.h"

#define SERVER_PORT_INTERCHANGE 43210
#define API_TOKEN               "MySeCrEtToKeN"

TEST(ClientCollectorTest, CollectClientMetrics)
{
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

    Barrier barrier(2);

    std::thread server_thread([&]() {
        comm::ConnServer server(comm::simpleTCPConfiguration(SERVER_PORT_INTERCHANGE));

        barrier.wait();

        auto server_conn = server.negotiate_protocol(server.accept());

        auto handle_query = [&](const std::string& expected, const nlohmann::json& resp) {
            auto recv = server_conn->recv_message();

            VDMS::protobufs::queryMessage cmd;
            cmd.ParseFromArray(recv.data(), recv.length());
            EXPECT_EQ(cmd.json(), expected);

            VDMS::protobufs::queryMessage res;
            res.set_json(resp.dump());
            std::basic_string< uint8_t > msg(res.ByteSizeLong(), 0);
            res.SerializeToArray(msg.data(), msg.length());
            server_conn->send_message(msg.data(), msg.length());
        };

        handle_query("[{\"Authenticate\":{\"token\":\"" API_TOKEN "\"}}]", R"([{
            "Authenticate": {
                "status": 0,
                "session_token": "123abc",
                "session_token_expires_in": 30,
                "refresh_token": "789xyz",
                "refresh_token_expires_in": 60
            }
        }])"_json);

        handle_query("[{\"GetMetrics\":{\"format\":\"json\"}}]", response);
    });

    nlohmann::json cfg_json;
    cfg_json[PA_CONFIG_VDMS_PORT_KEY] = SERVER_PORT_INTERCHANGE;
    cfg_json[PA_CONFIG_API_TOKEN_KEY] = API_TOKEN;
    PromConfig cfg(cfg_json);
    prometheus::Registry reg;
    ClientCollector cc(cfg, reg);

    barrier.wait();

    auto metrics = cc.Collect();

    server_thread.join();

    metrics::JsonWriter< nlohmann::json > writer;
    auto round_trip_json = writer.to_json(metrics);

    ASSERT_EQ(response[0]["GetMetrics"]["values"], round_trip_json);
}

TEST(ClientCollectorTest, UnableToConnect)
{
    nlohmann::json cfg_json;
    cfg_json[PA_CONFIG_VDMS_PORT_KEY] = SERVER_PORT_INTERCHANGE;
    PromConfig cfg(cfg_json);
    prometheus::Registry reg;
    ClientCollector cc(cfg, reg);

    auto metrics      = cc.Collect();
    auto self_metrics = reg.Collect();

    EXPECT_EQ(metrics.size(), 0);
    EXPECT_EQ(self_metrics.size(), 4);
}
