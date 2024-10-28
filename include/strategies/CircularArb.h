#pragma once
#include <vector>
#include <map>
#include <set>
#include <memory>
#include <optional>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <stdexcept>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "IStrategy.h"
#include "bnb/marketData/BookTickerMDFrame.h"
#include "utils/SymbolFilter.h"
#include "fin/Order.h"
#include "fin/Symbol.h"
#include "fin/Signal.h"
#include "bnb/marketConnection/BNBBroker.h"
#include "bnb/marketConnection/BNBFeeder.h"
#include "bnb/marketConnection/BNBMarketConnectionConfig.h"

const double FEE = 0.1;
const double RISK = 1.0;

struct CircularArbConfig {
    std::string startingCoin;
};

// Circular Arbitrage Strategy
class CircularArb : public IStrategy {
public:
    CircularArb(const CircularArbConfig& config, const BNBMarketConnectionConfig& mcConfig);
    virtual ~CircularArb() = default;

    // Override the methods from the IStrategy interface
    std::optional<Signal> onMarketData(const BookTickerMDFrame& data) override;
    void initialize() override;
    void shutdown() override;
    void run() override;

    static CircularArbConfig loadConfig(const std::string& configFile);
    
private:
    std::string startingCoin_;
    std::vector<std::vector<Order>> stratPaths_;
    std::set<std::string> stratSymbols_;
    std::map<std::string, BookTickerMDFrame> marketData_;
    std::map<std::string, double> balance_;

    BNBBroker broker_;
    BNBFeeder<BookTickerMDFrame> feeder_;

    // Helper methods
    std::vector<Order> getPossibleOrders(const std::string& coin, const std::vector<Symbol>& relatedSymbols);
    void computeArbitragePaths(const std::vector<Symbol>& fullPairsUniverse);
    std::optional<Signal> evaluatePath(const std::vector<Order>& path);
};
