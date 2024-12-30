#include "strategies/CircularArb.h"
#include <numeric>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <numeric>
#include <charconv>
#include "common/logger.hpp"
#include <ranges>

CircularArb::CircularArb(const CircularArbConfig& config, const BNBMarketConnectionConfig& mcConfig)
    : startingAsset_(config.startingAsset), broker_(mcConfig), feeder_(mcConfig){
    initialize();
} 

void CircularArb::initialize() {
    LOG_INFO("[STRATEGY] CircularArb initialized with starting coin: {}", startingAsset_);

    broker_.start();
    feeder_.start();

    LOG_INFO("[STRATEGY] Getting exchange information");
    std::string requestId = broker_.sendRequest(&BNBRequests::getExchangeInfo, {});
    nlohmann::json response = broker_.getResponseForId(requestId);
    auto exInfo = ExchangeInfo(response);

    std::vector<Symbol> symbolsList = exInfo.getSymbols();
    stratPaths_ = computeArbitragePaths(symbolsList, startingAsset_, 3);

    LOG_INFO("[STRATEGY] Getting account infromation");
    requestId = broker_.sendRequest(&BNBRequests::getAccountInformation);
    response = broker_.getResponseForId(requestId);

    const json& balances = response["result"]["balances"];
    for (const auto& balance : balances) {

        double val=0;
        std::string input = balance["free"].get<std::string>();
        auto [ptr, ec] = std::from_chars(input.data(), input.data() + input.size(), val);

        if (ec == std::errc::invalid_argument) {
            LOG_ERROR("Invalid argument on balance coversion from str to double {}", input);
        } else if (ec == std::errc::result_out_of_range) {
            LOG_ERROR("Out of range on balance coversion from str to double {}", input);
        }
        std::string asset = balance["asset"].get<std::string>();
        balance_[asset] = val;
    }

    LOG_INFO("[STRATEGY] Initializing market data");
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

    requestId = broker_.sendRequest(&BNBRequests::SymbolOrderBookTicker, {relatedSymbols.begin(), relatedSymbols.end()});
    response = broker_.getResponseForId(requestId);

    const json& data = response["result"];
    for (const auto& json_data : data) {
        BookTickerMDFrame dataFrame;
        dataFrame.symbol = json_data["symbol"].get<std::string>();
        dataFrame.bestBidPrice = std::stod(json_data["bidPrice"].get<std::string>());
        dataFrame.bestBidQty = std::stod(json_data["bidQty"].get<std::string>());
        dataFrame.bestAskPrice = std::stod(json_data["askPrice"].get<std::string>());
        dataFrame.bestAskQty = std::stod(json_data["askQty"].get<std::string>());
        marketData_[dataFrame.symbol] = dataFrame;
        LOG_DEBUG("Starting BookTicker : {}", dataFrame.to_str());
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
std::optional<Signal> CircularArb::evaluatePath(std::vector<Order>& path) {
    Order firstOrder = path[0];
    std::string pathStartingAsset = firstOrder.getStartingAsset();
    std::string signalDesc;
    std::string pathDescription = std::accumulate(
        path.begin(), 
        path.end(),
        std::string(),
        [](const std::string& acc, const Order& ord) { return acc.empty() ? ord.to_str() : acc + " " + ord.to_str(); });

    LOG_DEBUG("Evaluating path : {}", pathDescription);

    // Asset quantities
    double startingAssetQty = 0;
    double resultingAssetQty = RISK * balance_[pathStartingAsset];

    for (auto& order : path) {
        startingAssetQty = resultingAssetQty;

        if (startingAssetQty == 0) {
            LOG_DEBUG("starting asset qty for {} is null, cannot proceed with path evaluation.", order.getStartingAsset());
            return std::nullopt;
        }

        if (marketData_.find(order.getSymbol().to_str()) == marketData_.end())
        {
            LOG_DEBUG("Market data still unavailale for [{}]", order.getSymbol().to_str());
            return std::nullopt;
        }

        // base asset refers to the asset that is the quantity of a symbol.
        // quote asset refers to the asset that is the price of a symbol.
        // For the symbol XRPUSDC, USDC would be the quote asset.
        // For the symbol XRPUSDC, XRP would be the base asset.

        const BookTickerMDFrame& marketData = marketData_[order.getSymbol().to_str()];

        double orderPrice=0;
        double orderQty=0;

        if (order.getWay() == Way::SELL)
        {
            // sell to the bid 
            orderQty = order.getSymbol().getFilter().roundQty(startingAssetQty);
            resultingAssetQty = orderQty * marketData.bestBidPrice;
            orderPrice=marketData.bestBidPrice;
        }
        if (order.getWay() == Way::BUY)
        {
            // buy from the ask 
            orderQty = order.getSymbol().getFilter().roundQty(startingAssetQty/ marketData.bestAskPrice);
            resultingAssetQty = orderQty;
            orderPrice= marketData.bestAskPrice;
        }

        LOG_DEBUG("Transaction : {} {} -> {} {}", startingAssetQty, order.getStartingAsset(), resultingAssetQty, order.getResultingAsset());

        order.setPrice(orderPrice);
        order.setQty(orderQty);
        order.setType(OrderType::MARKET);

        //apply fee
        resultingAssetQty *= (1 - FEE / 100);
        LOG_DEBUG("Amound after fees {}", resultingAssetQty);

    }

    double pnl = resultingAssetQty - RISK * balance_[pathStartingAsset];
    if (pnl>0)
    {
        return Signal(path, pathDescription, pnl);
    }
    else
    {
        return std::nullopt;
    }
}

// Handle incoming market data
std::optional<Signal> CircularArb::onMarketData(const BookTickerMDFrame& data) {
    marketData_[data.symbol] = data;
    double maxPnl=0;
    std::optional<Signal> outSignal;
    for (auto& path : stratPaths_) {
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
                LOG_INFO("Detected a trading signal, theo PNL : {}, description : {}", sig->pnl, sig->description);
//                broker_.executeOrders(sig->orders);
                if (sig->pnl > 10)
                {
                    return;
                }
            }
        } catch (const std::exception& e) {
            LOG_ERROR("Error in Circular Arb loop: {}", e.what());
            break;
        }
    }
}
