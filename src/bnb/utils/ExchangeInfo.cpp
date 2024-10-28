// ExchangeInfo.cpp
#include "bnb/utils/ExchangeInfo.h"
#include <nlohmann/json.hpp>  // Include nlohmann json library
#include <stdexcept>
#include <iostream>

using json = nlohmann::json;

// Constructor to initialize from JSON string
ExchangeInfo::ExchangeInfo(const std::string& jsonData) {
    json root = json::parse(jsonData); // Parse JSON data

    // Parse symbols
    const json& symbolsJson = root["result"]["symbols"];
    for (const auto& symbolJson : symbolsJson) {
        SymbolJson symbol;
        symbol.name = symbolJson["symbol"].get<std::string>();
        symbol.status = symbolJson["status"].get<std::string>();
        symbol.baseAsset = symbolJson["baseAsset"].get<std::string>();
        symbol.quoteAsset = symbolJson["quoteAsset"].get<std::string>();
        
        // Store symbol filters
        for (const auto& filter : symbolJson["filters"]) {
            std::map<std::string, std::string> filterMap;
            for (const auto& [key, value] : filter.items()) {
                filterMap[key] = value.get<std::string>();
            }
            symbol.filters.push_back(filterMap);
        }

        symbols.push_back(symbol);
    }
}

// Method to get all symbols that are trading
std::vector<Symbol> ExchangeInfo::getSymbols() const {
    std::vector<Symbol> tradingSymbols;
    for (const auto& symbol : symbols) {
        if (symbol.status == "TRADING") {
            auto sym = Symbol(symbol.baseAsset, symbol.quoteAsset, symbol.name, createSymbolFilter(symbol.name));
            tradingSymbols.push_back(sym);
        }
    }
    return tradingSymbols;
}

std::vector<Symbol> ExchangeInfo::getRelatedSymbols(std::string asset) const {
    std::vector<Symbol> relatedSymbols;
    for (const auto& symbol : symbols) {
        if ((symbol.status == "TRADING") && ((symbol.baseAsset == asset) || (symbol.quoteAsset == asset))) {
            auto sym = Symbol(symbol.baseAsset, symbol.quoteAsset, symbol.name, createSymbolFilter(symbol.name));
            relatedSymbols.push_back(sym);
        }
    }
    return relatedSymbols;
}

// Factory method to create SymbolFilter for a specific symbol
SymbolFilter ExchangeInfo::createSymbolFilter(const std::string& symbolName) const {
    for (const auto& symbol : symbols) {
        if (symbol.name == symbolName) {
            PriceFilter pf = {0, 0, 0};
            LotSizeFilter lsf = {0, 0, 0};
            MarketLotSizeFilter mlsf = {0, 0, 0};
            NotionalFilter nf = {0, false, 0, false, 0};
            MinNotionalFilter mnf = {0, false, 0};
            MaxPositionFilter mpf = {0};

            // Fill filters
            for (const auto& filter : symbol.filters) {
                if (filter.at("filterType") == "PRICE_FILTER") {
                    pf.minPrice = std::stod(filter.at("minPrice"));
                    pf.maxPrice = std::stod(filter.at("maxPrice"));
                    pf.tickSize = std::stod(filter.at("tickSize"));
                } else if (filter.at("filterType") == "LOT_SIZE") {
                    lsf.minQty = std::stod(filter.at("minQty"));
                    lsf.maxQty = std::stod(filter.at("maxQty"));
                    lsf.stepSize = std::stod(filter.at("stepSize"));
                } else if (filter.at("filterType") == "MARKET_LOT_SIZE") {
                    mlsf.minQty = std::stod(filter.at("minQty"));
                    mlsf.maxQty = std::stod(filter.at("maxQty"));
                    mlsf.stepSize = std::stod(filter.at("stepSize"));
                } else if (filter.at("filterType") == "NOTIONAL") {
                    nf.minNotional = std::stod(filter.at("minNotional"));
                    nf.applyMinToMarket = filter.at("applyMinToMarket") == "true";
                    nf.maxNotional = std::stod(filter.at("maxNotional"));
                    nf.applyMaxToMarket = filter.at("applyMaxToMarket") == "true";
                    nf.avgPriceMins = std::stoi(filter.at("avgPriceMins"));
                } else if (filter.at("filterType") == "MIN_NOTIONAL") {
                    mnf.minNotional = std::stod(filter.at("minNotional"));
                    mnf.applyToMarket = filter.at("applyToMarket") == "true";
                    mnf.avgPriceMins = std::stoi(filter.at("avgPriceMins"));
                } else if (filter.at("filterType") == "MAX_POSITION") {
                    mpf.maxPosition = std::stod(filter.at("maxPosition"));
                }
            }

            // Create and return a SymbolFilter object
            return SymbolFilter(pf, lsf, mlsf, nf, mnf, mpf);
        }
    }
    throw std::invalid_argument("Symbol not found: " + symbolName);
}
