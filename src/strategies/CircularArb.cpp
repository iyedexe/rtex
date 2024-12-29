#include "strategies/CircularArb.h"

#include <algorithm>
#include <iostream>
#include <chrono>
#include <numeric>

#include "common/logger.hpp"

CircularArb::CircularArb(const CircularArbConfig& config, const BNBMarketConnectionConfig& mcConfig)
    : startingAsset_(config.startingAsset), broker_(mcConfig), feeder_(mcConfig){
    initialize();
} 

void CircularArb::initialize() {
    LOG_INFO("[STRATEGY] CircularArb initialized with starting coin: {}", startingAsset_);

    broker_.start();
    feeder_.start();

    LOG_INFO("[STRATEGY] Waiting for exchange info response...");
    std::string requestId = broker_.sendRequest(&BNBRequests::getExchangeInfo, {});
    nlohmann::json response = broker_.getResponseForId(requestId);
    auto exInfo = ExchangeInfo(response);
    
    LOG_INFO("[STRATEGY] Getting account infromation");
    requestId = broker_.sendRequest(&BNBRequests::getAccountInformation);
    response = broker_.getResponseForId(requestId);

    const json& balances = response["result"]["balances"];
    for (const auto& balance : balances) {
        LOG_INFO(
            "Balance: {}, free: {}, locked: {}", 
            balance["asset"].get<std::string>(),
            balance["free"].get<std::string>(),
            balance["locked"].get<std::string>()
        );
    }


    std::vector<Symbol> symbolsList = exInfo.getSymbols();
    stratPaths_ = computeArbitragePaths(symbolsList, startingAsset_, 3);

    std::set<std::string> relatedSymbols;
    for (auto path: stratPaths_)
    {
        std::string pathDescription;
        for (auto order: path)
        {
            relatedSymbols.insert(order.getSymbol().getSymbol());
            pathDescription += order.to_str() + " ";
        }
        LOG_DEBUG("[STRATEGY] Arbitrage path : {}", pathDescription);
    }
    feeder_.subscribeToTickers({relatedSymbols.begin(), relatedSymbols.end()});
}

void CircularArb::shutdown() {
    LOG_INFO("[STRATEGY] Shutting down Triangular Arbitrage Strategy...");
    broker_.stop();
    feeder_.stop();
}

CircularArbConfig CircularArb::loadConfig(const std::string& configFile){
    CircularArbConfig config;
    boost::property_tree::ptree pt;

    try {
        boost::property_tree::ini_parser::read_ini(configFile, pt);

        config.startingAsset = pt.get<std::string>("CIRCULAR_ARB_STRATEGY.startingAsset");
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
std::vector<std::vector<Order>> CircularArb::computeArbitragePaths(const std::vector<Symbol>& symbolsList, const std::string& startingAsset, int arbitrageDepth) {
    LOG_INFO("[STRATEGY] Computing arbitrage paths...");
    std::vector<std::vector<Order>> stratPaths; 
    auto firstOrders = getPossibleOrders(startingAsset, symbolsList);
    for (const auto& order : firstOrders) {
        stratPaths.push_back({order});
    }

    for (int i = 0; i < arbitrageDepth - 1; ++i) {
        std::vector<std::vector<Order>> paths;
        for (const auto& path : stratPaths) {
            Order lastOrder = path.back();
            std::string resultingCoin = (lastOrder.getWay() == Way::SELL) ? lastOrder.getSymbol().getQuote() : lastOrder.getSymbol().getBase();
            std::vector<Symbol> unusedSymbols;
            for (const auto& symbol : symbolsList) {
                if (std::find_if(path.begin(), path.end(), [&symbol](const Order& order) { return order.getSymbol() == symbol; }) == path.end()) {
                    unusedSymbols.push_back(symbol);
                }
            }
            std::vector<Order> possibleNextOrders = getPossibleOrders(resultingCoin, unusedSymbols);
            for (const auto& nextOrder : possibleNextOrders) {
                std::string resultingCoin = (nextOrder.getWay() == Way::SELL) ? nextOrder.getSymbol().getQuote() : nextOrder.getSymbol().getBase();
                if ((i==arbitrageDepth-2) & (resultingCoin != startingAsset))
                {
                    continue;
                }
                auto newPath = path;
                newPath.push_back(nextOrder);
                paths.push_back(newPath);
            }
        }
        stratPaths = paths;
    }
    LOG_INFO("[STRATEGY] Number of arbitrage paths : {} of depth {}, starting from asset {}", stratPaths.size(), arbitrageDepth, startingAsset);
    return stratPaths;
}

// Evaluate potential arbitrage path profitability
std::optional<Signal> CircularArb::evaluatePath(const std::vector<Order>& path) {
    Order firstOrder = path[0];
    std::string pathStartingAsset = (firstOrder.getWay() == Way::BUY) ? firstOrder.getSymbol().getQuote() : firstOrder.getSymbol().getBase();
    std::string signalDesc;

    double orderStartingAmount = RISK * balance_[pathStartingAsset];
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

    double pnl = orderResultingAmount - RISK * balance_[pathStartingAsset];
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
                LOG_INFO("Detected a trading signal, theo PNL : {}", sig->pnl);
//                broker_.executeOrders(sig->orders);
            }
        } catch (const std::exception& e) {
            LOG_ERROR("Error in Circular Arb loop: {}", e.what());
            break;
        }
    }
}
