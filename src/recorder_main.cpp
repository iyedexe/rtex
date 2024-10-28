#include "BNBRecorder.h"
#include "bnb/marketData/MarketDataFrame.h"
#include "bnb/marketConnection/BNBMarketConnectionConfig.h"
#include <iostream>
#include <string>
#include <getopt.h>

void printUsage(const std::string& programName) {
    std::cout << "Usage: " << programName << " --symbol <symbol> --date <YYYY-MM-DD> --configfile <path_to_ini>" << std::endl;
    std::cout << "       --symbol: The symbol to record (e.g., BTCUSDT). Use 'all' to record all symbols." << std::endl;
    std::cout << "       --date: The date to start recording (YYYY-MM-DD)." << std::endl;
    std::cout << "       --configfile: Path to the configuration INI file." << std::endl;
}

int main(int argc, char* argv[]) {
    std::string symbol;
    std::string date;
    std::string configFile;

    // Option parsing
    static struct option long_options[] = {
        {"symbol", required_argument, 0, 's'},
        {"date", required_argument, 0, 'd'},
        {"configfile", required_argument, 0, 'c'},
        {0, 0, 0, 0}
    };

    int option_index = 0;
    int c;
    while ((c = getopt_long(argc, argv, "s:d:c:", long_options, &option_index)) != -1) {
        switch (c) {
            case 's':
                symbol = optarg;
                break;
            case 'd':
                date = optarg;
                break;
            case 'c':
                configFile = optarg;
                break;
            case '?':
                printUsage(argv[0]);
                return 1;
            default:
                printUsage(argv[0]);
                return 1;
        }
    }

    if (symbol.empty() || date.empty() || configFile.empty()) {
        std::cerr << "Error: --symbol, --date, and --configfile parameters are required." << std::endl;
        printUsage(argv[0]);
        return 1;
    }

    try {
        BNBMarketConnectionConfig config = loadConfig(configFile);

        auto recorder = std::make_unique<BNBRecorder>(config , date, symbol);
        recorder->run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
