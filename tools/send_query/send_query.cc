/**
 *
 * @copyright Copyright (c) 2020 ApertureData Inc.
 *
 */

#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#include <nlohmann/json.hpp>
#include <glog/logging.h>

#include "aperturedb/VDMSClient.h"

template< typename T >
inline std::string get_pretty_json(T input)
{
    return nlohmann::json::parse(input).dump();
}

int main(int argc, char const *argv[])
{
    static bool _always_false{false};
    if (_always_false) {
        // This will never run, but it needs to be here to force the linker to link glog.
        // Otherwise the linker will fail due to missing VLOG symbols in libcomm.
        LOG(INFO) << "wtf";
    }

    if (argc != 6) {
        std::cout << "Usage: send_query db_ip db_port username password query_file(json)";
        std::cout << std::endl;
        return 0;
    }

    std::string ip(argv[1]);
    int port = std::stoi(std::string(argv[2]));
    std::string username(argv[3]);
    std::string password(argv[4]);
    std::string filename(argv[5]);

    auto query = get_pretty_json(std::ifstream(filename));

    std::cout << "Connecting to server " << ip << ":"
                                         << port << "..." << std::endl;

    VDMS::VDMSClient connector(username, password, VDMS::VDMSClientConfig(ip, port));

    std::cout << "Connection successful." << std::endl;

    std::cout << "Sending query:" << std::endl;
    std::cout << query << std::endl;

    // Run query
    auto response = connector.query(query);

    std::cout << "Response recieved:" << std::endl;
    std::cout << get_pretty_json(response.json) << std::endl;

    std::cout << "Total returned blobs: " << response.blobs.size() << std::endl;

    return 0;
}
