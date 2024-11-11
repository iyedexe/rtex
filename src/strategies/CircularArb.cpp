#include "strategies/CircularArb.h"

#include <algorithm>
#include <iostream>
#include <chrono>
#include <numeric>

#include "common/logger.hpp"

CircularArb::CircularArb(const CircularArbConfig& config, const BNBMarketConnectionConfig& mcConfig)
    : startingCoin_(config.startingCoin), broker_(mcConfig), feeder_(mcConfig){
    initialize();
} 

void CircularArb::initialize() {
    LOG_INFO("CircularArb initialized with starting coin: {}", startingCoin_);

    broker_.start();
    feeder_.start();
    std::string requestId = broker_.sendRequest(&BNBRequests::getExchangeInfo, {});

    LOG_INFO("Waiting for exchange info response...");
    nlohmann::json response = broker_.getResponseForId(requestId);

    auto exInfo = ExchangeInfo(response);

    std::vector<Symbol> relatedSym = exInfo.getRelatedSymbols(startingCoin_);
    computeArbitragePaths(relatedSym);
}

void CircularArb::shutdown() {
    LOG_INFO("Shutting down Triangular Arbitrage Strategy...");
    broker_.stop();
    feeder_.stop();
}

CircularArbConfig CircularArb::loadConfig(const std::string& configFile){
    CircularArbConfig config;
    boost::property_tree::ptree pt;

    try {
        boost::property_tree::ini_parser::read_ini(configFile, pt);

        config.startingCoin = pt.get<std::string>("CIRCULAR_ARB_STRATEGY.startingCoin");
    } catch (const boost::property_tree::ini_parser_error& e) {
        throw std::runtime_error("Failed to load config file: " + std::string(e.what()));
    } catch (const boost::property_tree::ptree_bad_path& e) {
        throw std::runtime_error("Missing parameter in config file: " + std::string(e.what()));
    }

    return config;
}

// Determine all possible orders where coin can be used
std::vector<Order> CircularArb::getPossibleOrders(const std::string& coin, const std::vector<Symbol>& relatedSymbols) {
    std::vector<Order> orders;
    for (const auto& symbol : relatedSymbols) {
        if (coin == symbol.getBase()) {
            orders.emplace_back(symbol, Way::SELL);
        } else if (coin == symbol.getQuote()) {
            orders.emplace_back(symbol, Way::BUY);
        }
    }
    return orders;
}

// Compute all potential triangular arbitrage paths
void CircularArb::computeArbitragePaths(const std::vector<Symbol>& relatedSymbols) {
    LOG_INFO("Computing arbitrage paths...");

    auto firstOrders = getPossibleOrders(startingCoin_, relatedSymbols);
    for (const auto& order : firstOrders) {
        stratPaths_.push_back({order});
    }

    const int arbitrageDepth = 3;
    for (int i = 0; i < arbitrageDepth - 1; ++i) {
        std::vector<std::vector<Order>> paths;
        for (const auto& path : stratPaths_) {
            Order lastOrder = path.back();
            std::string resultingCoin = lastOrder.getWay() == Way::SELL ? lastOrder.getSymbol().getQuote() : lastOrder.getSymbol().getBase();
            std::vector<Symbol> unusedSymbols;
            for (const auto& symbol : relatedSymbols) {
                if (std::find_if(path.begin(), path.end(), [&symbol](const Order& order) { return order.getSymbol() == symbol; }) == path.end()) {
                    unusedSymbols.push_back(symbol);
                }
            }
            std::vector<Order> possibleNextOrders = getPossibleOrders(resultingCoin, unusedSymbols);
            for (const auto& nextOrder : possibleNextOrders) {
                std::string resultingCoin = nextOrder.getWay() == Way::SELL ? nextOrder.getSymbol().getQuote() : nextOrder.getSymbol().getBase();
                if ((i == arbitrageDepth -1) && (resultingCoin != startingCoin_))
                {
                    continue;
                }
                auto newPath = path;
                newPath.push_back(nextOrder);
                paths.push_back(newPath);
            }
        }
        stratPaths_ = paths;
    }

    //LOG_INFO("Found " + std::to_string(stratPaths_.size()) + " possible paths for arbitrage starting on " + startingCoin_);
}

// Evaluate potential arbitrage path profitability
std::optional<Signal> CircularArb::evaluatePath(const std::vector<Order>& path) {
    Order firstOrder = path[0];
    std::string pathStartingCoin = firstOrder.getWay() == Way::BUY ? firstOrder.getSymbol().getQuote() : firstOrder.getSymbol().getBase();
    std::string signalDesc;

    double orderStartingAmount = RISK * balance_[pathStartingCoin];
    double orderResultingAmount;
    for (const auto& order : path) {
        if (orderStartingAmount == 0) {
            LOG_ERROR("Available amount is zero, cannot proceed with path evaluation");
            return std::nullopt;
        }
        const BookTickerMDFrame& marketData = marketData_[order.getSymbol().to_str()];
        // buy or sell the base
        if (order.getWay() == Way::SELL)
        {
            // sell to the bid 
            // bid is how much quote to get for selling 1 base
            orderResultingAmount = orderStartingAmount * marketData.bestBidPrice;
        }
        if (order.getWay() == Way::BUY)
        {
            // buy from the ask 
            // ask is how much quote to spend to get 1 base
            orderResultingAmount = orderStartingAmount / marketData.bestAskPrice;
        }

        //apply fee
        orderResultingAmount *= (1 - FEE / 100);
        
        // round using tick size
        orderResultingAmount = order.getSymbol().getFilter().roundPrice(orderResultingAmount);
        
        orderStartingAmount = orderResultingAmount;

        //signalDesc += "symbol=[" + order.getSymbol() + "], way=[" + std::to_string(order.getWay()) + "], price=[" + std::to_string(price) + "]; ";
    }

    double pnl = orderResultingAmount - RISK * balance_[pathStartingCoin];
    //LOG_INFO("Arbitrage opportunity found: " + signalDesc + "PNL: " + std::to_string(pnl));
}

// Handle incoming market data
std::optional<Signal> CircularArb::onMarketData(const BookTickerMDFrame& data) {
    marketData_[data.symbol] = data;
    double maxPnl=0;
    std::optional<Signal> outSignal;
    for (const auto& path : stratPaths_) {
        if (std::any_of(path.begin(), path.end(), [&data](const Order& order) { return order.getSymbol().to_str() == data.symbol; })) {
            std::optional<Signal> sig = evaluatePath(path);
            if ((sig.has_value()) && (sig->pnl > maxPnl))
            {
                outSignal = sig;
            }
        }
    }
    return outSignal;
}


void CircularArb::run() {
    while (true) {
        try {
            auto update = feeder_.getUpdate();
            std::optional<Signal> sig = onMarketData(update);
            if (sig.has_value())
            {
//                broker_.executeOrders(sig->orders);
            }
        } catch (const std::exception& e) {
            LOG_ERROR("Error in Circular Arb loop: {}", e.what());
            break;
        }
    }
}
