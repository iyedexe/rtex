#pragma once

#include "bnb/marketConnection/BNBBroker.h"
#include "bnb/marketConnection/BNBFeeder.h"
#include "bnb/marketConnection/BNBMarketConnectionConfig.h"
#include "common/Scheduler.h"
#include "bnb/utils/BNBRequests.h"
#include "common/RecorderMonitor.h"

#include <string>
#include <unordered_map>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <cctype>    

namespace fs = std::filesystem;

class BNBRecorder {
public:
    BNBRecorder(const BNBMarketConnectionConfig& config, const std::string& date, const std::string& symbol);
    ~BNBRecorder();

    void run();

private:

    bool startComponents();
    std::vector<std::string> getSubscriptionList(const std::string& symbol);

    void recordData(const std::string& ticker, const std::string& data);
    void openFileForSymbol(const std::string& ticker);
    void closeFiles();

    Scheduler scheduler_;
    RecorderMonitor monitor_;
    BNBBroker broker_;
    BNBFeeder<BookTickerMDFrame> feeder_;

    std::string date_;
    std::string symbol_;
    std::unordered_map<std::string, std::ofstream> symbol_file_map_;
};