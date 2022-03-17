/**
 *
 * @copyright Copyright (c) 2020 ApertureData Inc.
 *
 */

#include <string>
#include <vector>
#include <iostream>
#include <fstream>

// Json parsing files
#include <jsoncpp/json/value.h>
#include <jsoncpp/json/writer.h>
#include <jsoncpp/json/reader.h>

#include "VDMSClient.h"


std::string get_pretty_json(std::string& json_str)
{
    Json::StyledWriter wr;

    Json::Value root;
    Json::Reader reader;
    bool parsingSuccessful = reader.parse(json_str, root);
    if ( !parsingSuccessful ) {
        std::cout << "Error parsing json_string" << std::endl;
        exit(0);
    }

    return wr.write(root);
}

int main(int argc, char const *argv[])
{

    if (argc != 4) {
        std::cout << "Usage: send_query db_ip db_port query_file(json)";
        std::cout << std::endl;
        return 0;
    }

    std::string ip(argv[1]);
    int port = std::stoi(std::string(argv[2]));
    std::string filename(argv[3]);

    std::string query;

    std::ifstream ifs(filename);
    while (ifs.good()) {
        std::string word;
        ifs >> word;
        if (ifs.eof()) {
            break;
        }

       query += word + " ";
    }

    query = get_pretty_json(query);

    std::cout << "Connecting to server " << ip << ":"
                                         << port << "..." << std::endl;
    VDMS::VDMSClient connector(ip, port);
    std::cout << "Connection successful." << std::endl;

    Json::StyledWriter wr;

    std::cout << "Sending query:" << std::endl;
    std::cout << query << std::endl;

    // Run query
    auto response = connector.query(query);

    std::cout << "Response recieved:" << std::endl;
    std::cout << get_pretty_json(response.json) << std::endl;

    std::cout << "Total returned blobs: " << response.blobs.size() << std::endl;

    return 0;
}
