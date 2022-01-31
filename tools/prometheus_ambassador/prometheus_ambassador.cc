/**
 *
 * @copyright Copyright (c) 2022 ApertureData Inc.
 *
 */

#include <iostream>

#include "PromServer.h"
#include "PromConfig.h"

int main(int argc, char **argv)
{
    std::string config_file("config.json");
    if (argc == 3 && std::string(argv[1]) == "-cfg") {
        config_file = argv[2];
    }
    else if (argc != 1) {
        std::cout << "Usage: " << argv[0] << " [-cfg <config_file>]" << std::endl;
        exit(0);
    }
    auto config = PromConfig::load(config_file);
    PromServer server(config);
    server.run();
    std::cout << "Bye" << std::endl;
    return 0;
}
