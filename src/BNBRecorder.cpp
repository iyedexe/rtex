#include "BNBRecorder.h"
#include "common/logger.hpp"

BNBRecorder::BNBRecorder(const BNBMarketConnectionConfig& config, const std::string& date, const std::string& symbol)
    : broker_(config), feeder_(config), date_(date), symbol_(symbol), scheduler_(date), monitor_(date){
    LOG_INFO("[RECORDER] Initialized with date: {} and symbol: {}", date_, symbol_);
}

BNBRecorder::~BNBRecorder() {
    LOG_INFO("[RECORDER] Shutting down BNBRecorder.");
    closeFiles();
    broker_.stop();
    feeder_.stop();
    monitor_.stop();
}

bool BNBRecorder::startComponents() {
    try {
        broker_.start();
        feeder_.start();
        monitor_.start();
        return true;
    } catch (const std::exception& e) {
        LOG_CRITICAL("[RECORDER] Exception while starting broker/feeder: {}", e.what());
        return false;
    }
}

std::vector<std::string> BNBRecorder::getSubscriptionList(const std::string& symbol)
{
    std::vector<std::string> subscriptionList;
    if (symbol_ == "all") 
    {
        std::string requestId = broker_.sendRequest(&BNBRequests::getExchangeInfo, {});
        LOG_INFO("[RECORDER] Waiting for exchange info response...");

        nlohmann::json response = broker_.getResponseForId(requestId);
        if (response.contains("result")) {
            for (const auto& symbolInfo : response["result"]["symbols"]) {
                if (symbolInfo["status"] != "TRADING")
                    continue;
                std::string symbol = symbolInfo["symbol"];
                std::transform(symbol.begin(), symbol.end(), symbol.begin(), ::tolower);
                subscriptionList.push_back(symbol);
            }
        }
        LOG_INFO("[RECORDER] Subscribing to all symbols...");
    }
    else
    {
        subscriptionList = {symbol_};
        LOG_INFO("[RECORDER] Subscribing to symbol: {}", symbol_);
    }
    return subscriptionList;
}

void BNBRecorder::run() {
    LOG_INFO("[RECORDER] Recorder started.");
    
    if (!scheduler_.waitStart()) return;    
    if (!startComponents()) return;
    
    auto subscriptionList = getSubscriptionList(symbol_);
    auto subscribedTickerCount = feeder_.subscribeToTickers(subscriptionList);

    auto lastLogTime = std::chrono::system_clock::now();
    auto stopTime = scheduler_.getStopTime();
    auto startTime = scheduler_.getStartTime();
    while (std::chrono::system_clock::now() < stopTime) {
        try {
            auto dataFrame = feeder_.getUpdate();
            std::chrono::seconds timeToStart = scheduler_.timeUntil(startTime);
            std::chrono::seconds timeToEnd = scheduler_.timeUntil(stopTime);
            monitor_.updateMetrics(timeToStart.count(), timeToEnd.count(), subscribedTickerCount, dataFrame.symbol);
            recordData(dataFrame.symbol, dataFrame.to_str());

            auto now = std::chrono::system_clock::now();
            if (std::chrono::duration_cast<std::chrono::minutes>(now - lastLogTime).count() >= 30) {

                auto now_c = std::chrono::system_clock::to_time_t(now);
                std::ostringstream oss;
                oss << std::put_time(std::gmtime(&now_c), "%Y-%m-%d %H:%M:%S");
                LOG_INFO("[RECORDER] Periodic log - Current time: {}, Total updates: {}", oss.str(), monitor_.getUpdatesCount());
                
                lastLogTime = now;
            }

        } catch (const std::exception& e) {
            LOG_ERROR("[RECORDER] Error in recorder loop: {}", e.what());
            break;
        }
    }

    LOG_INFO("[RECORDER] Target date has ended. Stopping recorder.");
}

void BNBRecorder::recordData(const std::string& ticker, const std::string& data) {
    if (symbol_file_map_.find(ticker) == symbol_file_map_.end()) {
        openFileForSymbol(ticker);
    }
    symbol_file_map_[ticker] << data << std::endl;
}

void BNBRecorder::openFileForSymbol(const std::string& ticker) {
    std::string base_dir = "data/";
    std::string symbol_dir = base_dir + ticker + "/" + date_;
    std::string filename = symbol_dir + "/book.csv";
    bool file_exists = fs::exists(filename);

    if (!file_exists) {
        fs::create_directories(symbol_dir);
        symbol_file_map_[ticker] = std::ofstream(filename, std::ios_base::app);
        LOG_INFO("[RECORDER] File for symbol {} did not exist, creating and writing header.", ticker);
        symbol_file_map_[ticker] << BookTickerMDFrame::getHeader() << std::endl;
    } else {
        symbol_file_map_[ticker] = std::ofstream(filename, std::ios_base::app);
        LOG_INFO("[RECORDER] Appending data to existing file for symbol {}: {}", ticker, filename);
    }

    LOG_INFO("[RECORDER] Opened file for symbol {}: {}", ticker, filename);
}

void BNBRecorder::closeFiles() {
    for (auto& pair : symbol_file_map_) {
        if (pair.second.is_open()) {
            LOG_INFO("[RECORDER] Closing file for symbol: {}", pair.first);
            pair.second.close();
        }
    }
}