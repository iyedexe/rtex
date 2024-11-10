#include "BNBRecorder.h"
#include "common/logger.hpp"

BNBRecorder::BNBRecorder(const BNBMarketConnectionConfig& config, const std::string& date, const std::string& symbol)
    : broker_(config), feeder_(config), date_(date), symbol_(symbol), scheduler_(date), monitor_(date){
    LOG_INFO("BNBRecorder initialized with date: {} and symbol: {}", date_, symbol_);
}

BNBRecorder::~BNBRecorder() {
    LOG_INFO("Shutting down BNBRecorder.");
    closeFiles();
    broker_.stop();
    feeder_.stop();
    monitor_.stop();
}

void BNBRecorder::run() {
    LOG_INFO("Recorder started.");
    
    if (scheduler_.isFuture()) {
        auto startTime = scheduler_.startTime();
        std::chrono::seconds wait_duration = scheduler_.timeUntil(startTime);
        LOG_INFO("Date {} is in the future. Scheduling start at {} ({}), which is in {}.", 
                     date_, scheduler_.formattedTime(startTime), scheduler_.formattedTime(startTime, true), 
                     scheduler_.humanReadable(wait_duration));
        std::this_thread::sleep_for(wait_duration);

    } else if (scheduler_.isToday()) {
        LOG_INFO("Recording data for today: {}", date_);
    } else {
        LOG_ERROR("The date {} is in the past. Exiting.", date_);
        return;
    }
    auto endTime = scheduler_.endTime();
    std::chrono::seconds wait_duration = scheduler_.timeUntil(endTime);
    LOG_INFO("Scheduling stop at {} ({}), which is in {}.", 
                    scheduler_.formattedTime(endTime), scheduler_.formattedTime(endTime, true), 
                    scheduler_.humanReadable(wait_duration));

    try {
        broker_.start();
        feeder_.start();
    } catch (const std::exception& e) {
        LOG_CRITICAL("Exception while starting broker/feeder: {}", e.what());
        return;
    }

    std::vector<std::string> subscriptionList;
    if (symbol_ == "all") 
    {
        std::string requestId = broker_.sendRequest(&BNBRequests::getExchangeInfo, {});
        LOG_INFO("Waiting for exchange info response...");

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
        LOG_INFO("Subscribing to all symbols...");
    }
    else
    {
        subscriptionList = {symbol_};
        LOG_INFO("Subscribing to symbol: {}", symbol_);
    }

    auto subscribedTickerCount = feeder_.subscribeToTickers(subscriptionList);
    monitor_.start();

    auto lastLogTime = std::chrono::system_clock::now();
    while (std::chrono::system_clock::now() < endTime) {
        try {
            auto dataFrame = feeder_.getUpdate();
            std::chrono::seconds timeToStart = scheduler_.timeUntil(scheduler_.startTime());
            std::chrono::seconds timeToEnd = scheduler_.timeUntil(endTime);
            monitor_.updateMetrics(timeToStart.count(), timeToEnd.count(), subscribedTickerCount, dataFrame.symbol);
            recordData(dataFrame.symbol, dataFrame.to_str());

            auto now = std::chrono::system_clock::now();
            if (std::chrono::duration_cast<std::chrono::minutes>(now - lastLogTime).count() >= 30) {

                auto now_c = std::chrono::system_clock::to_time_t(now);
                std::ostringstream oss;
                oss << std::put_time(std::gmtime(&now_c), "%Y-%m-%d %H:%M:%S");
                LOG_INFO("Periodic log - Current time: {}, Total updates: {}", oss.str(), monitor_.getUpdatesCount());
                
                lastLogTime = now;
            }

        } catch (const std::exception& e) {
            LOG_ERROR("Error in recorder loop: {}", e.what());
            break;
        }
    }

    LOG_INFO("Target date has ended. Stopping recorder.");
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
        LOG_INFO("File for symbol {} did not exist, creating and writing header.", ticker);
        symbol_file_map_[ticker] << BookTickerMDFrame::getHeader() << std::endl;
    } else {
        symbol_file_map_[ticker] = std::ofstream(filename, std::ios_base::app);
        LOG_INFO("Appending data to existing file for symbol {}: {}", ticker, filename);
    }

    LOG_INFO("Opened file for symbol {}: {}", ticker, filename);
}

void BNBRecorder::closeFiles() {
    for (auto& pair : symbol_file_map_) {
        if (pair.second.is_open()) {
            LOG_INFO("Closing file for symbol: {}", pair.first);
            pair.second.close();
        }
    }
}