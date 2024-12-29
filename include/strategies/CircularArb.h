#pragma once
#include <vector>
#include <map>
#include <set>
#include <memory>
#include <optional>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <stdexcept>

#include "IStrategy.h"
#include "bnb/marketData/BookTickerMDFrame.h"
#include "bnb/marketData/MarketDataFrame.h"
#include "bnb/utils/SymbolFilter.h"
#include "fin/Order.h"
#include "fin/Symbol.h"
#include "fin/Signal.h"
#include "bnb/marketConnection/BNBBroker.h"
#include "bnb/marketConnection/BNBFeeder.h"
#include "bnb/marketConnection/BNBMarketConnectionConfig.h"

#include "bnb/utils/BNBRequests.h"
#include "bnb/utils/ExchangeInfo.h"

const double FEE = 0.1;
const double RISK = 1.0;

struct CircularArbConfig {
    std::string startingAsset;
};

class CircularArb : public IStrategy {
public:
    CircularArb(const CircularArbConfig& config, const BNBMarketConnectionConfig& mcConfig);
    virtual ~CircularArb() = default;

    std::optional<Signal> onMarketData(const BookTickerMDFrame& data) override;
    void initialize() override;
    void shutdown() override;
    void run() override;

    static CircularArbConfig loadConfig(const std::string& configFile);
    
private:
    std::string startingAsset_;
    std::vector<std::vector<Order>> stratPaths_;
    std::set<std::string> stratSymbols_;
    std::map<std::string, BookTickerMDFrame> marketData_;
    std::map<std::string, double> balance_;

    BNBBroker broker_;
    BNBFeeder<BookTickerMDFrame> feeder_;

    std::vector<Order> getPossibleOrders(const std::string& coin, const std::vector<Symbol>& relatedSymbols);
    std::vector<std::vector<Order>> computeArbitragePaths(const std::vector<Symbol>& symbolsList, const std::string& startingAsset, int arbitrageDepth);
    std::optional<Signal> evaluatePath(const std::vector<Order>& path);
};
