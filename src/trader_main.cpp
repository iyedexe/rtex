#include "strategies/CircularArb.h"
#include <iostream>
#include <string>
#include <getopt.h>
#include "common/logger.hpp"

void printUsage(const std::string& programName) {
    std::cout << "Usage: " << programName << " --configfile <path_to_ini> --strategy <strategy_name>" << std::endl;
    std::cout << "       --configfile: Path to the configuration INI file." << std::endl;
    std::cout << "       --strategy  : Name of the trading strategy to run." << std::endl;
}

int main(int argc, char* argv[]) {
    std::string configFile;
    std::string strategyName;

    // Option parsing
    static struct option long_options[] = {
        {"configfile", required_argument, 0, 'c'},
        {"strategy", required_argument, 0, 's'},
        {0, 0, 0, 0}
    };

    int option_index = 0;
    int c;
    while ((c = getopt_long(argc, argv, "s:c:", long_options, &option_index)) != -1) {
        switch (c) {
            case 'c':
                configFile = optarg;
                break;
            case 's':
                strategyName = optarg;
                break;
            case '?':
                printUsage(argv[0]);
                return 1;
            default:
                printUsage(argv[0]);
                return 1;
        }
    }

    if (configFile.empty()) {
        std::cerr << "Error: --strategy and --configfile parameters are required." << std::endl;
        printUsage(argv[0]);
        return 1;
    }

    auto mcConfig = loadConfig(configFile);

    std::unique_ptr<IStrategy> strategy;
    if (strategyName == "CircularArb") 
    {
        auto config = CircularArb::loadConfig(configFile);
        strategy = std::make_unique<CircularArb>(config, mcConfig);
    }

    try {
        strategy->run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
