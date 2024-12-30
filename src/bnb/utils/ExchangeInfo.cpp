#include "common/logger.hpp"
#include "bnb/utils/ExchangeInfo.h"

ExchangeInfo::ExchangeInfo(const json& jsonData) {
    try
    {
        const json& symbolsJson = jsonData["result"]["symbols"];
        for (const auto& symbolJson : symbolsJson) {
            if (symbolJson["status"].get<std::string>() != "TRADING")
            {
                continue;
            }
            symbols_.push_back(
                Symbol(
                    symbolJson["baseAsset"].get<std::string>(),
                    symbolJson["quoteAsset"].get<std::string>(),
                    symbolJson["symbol"].get<std::string>(),
                    createSymbolFilter(symbolJson["filters"])
                )
            );
        }
    }
    catch(const std::exception& e)
    {
        LOG_ERROR("Error parsing exchange info : {}", e.what());
    }
    
}

std::vector<Symbol> ExchangeInfo::getSymbols() const {
    return symbols_;
}

std::vector<Symbol> ExchangeInfo::getRelatedSymbols(std::string asset) const {
    std::vector<Symbol> relatedSymbols;
    for (const auto& symbol : symbols_) {
        if ((symbol.getBase() == asset) || (symbol.getQuote() == asset)) {
            relatedSymbols.push_back(symbol);
        }
    }
    return relatedSymbols;
}

SymbolFilter ExchangeInfo::createSymbolFilter(const json& filterJson) const {
    PriceFilter pf = {0, 0, 0};
    LotSizeFilter lsf = {0, 0, 0};
    MarketLotSizeFilter mlsf = {0, 0, 0};
    NotionalFilter nf = {0, false, 0, false, 0};
    MinNotionalFilter mnf = {0, false, 0};
    MaxPositionFilter mpf = {0};

    for (const auto& filter : filterJson) {
        LOG_DEBUG("Parsing filter {}, {}", filter["filterType"].get<std::string>(), filter.dump());
        if (filter["filterType"].get<std::string>() == "PRICE_FILTER") {
            pf.maxPrice = std::stod(filter["maxPrice"].get<std::string>());
            pf.minPrice = std::stod(filter["minPrice"].get<std::string>());
            pf.tickSize = std::stod(filter["tickSize"].get<std::string>());
        } else if (filter["filterType"].get<std::string>() == "LOT_SIZE") {
            lsf.minQty = std::stod(filter["minQty"].get<std::string>());
            lsf.maxQty = std::stod(filter["maxQty"].get<std::string>());
            lsf.stepSize = std::stod(filter["stepSize"].get<std::string>());
        } else if (filter["filterType"].get<std::string>() == "MARKET_LOT_SIZE") {
            mlsf.minQty = std::stod(filter["minQty"].get<std::string>());
            mlsf.maxQty = std::stod(filter["maxQty"].get<std::string>());
            mlsf.stepSize = std::stod(filter["stepSize"].get<std::string>());
        } else if (filter["filterType"].get<std::string>() == "NOTIONAL") {
            nf.minNotional = std::stod(filter["minNotional"].get<std::string>());
            nf.applyMinToMarket = filter["applyMinToMarket"].get<bool>();
            nf.maxNotional = std::stod(filter["maxNotional"].get<std::string>());
            nf.applyMaxToMarket = filter["applyMaxToMarket"].get<bool>();
            nf.avgPriceMins = filter["avgPriceMins"].get<int>();
        } else if (filter["filterType"].get<std::string>() == "MIN_NOTIONAL") {
            mnf.minNotional = std::stod(filter["minNotional"].get<std::string>());
            mnf.applyToMarket = filter["applyToMarket"].get<bool>();
            mnf.avgPriceMins = std::stoi(filter["avgPriceMins"].get<std::string>());
        } else if (filter["filterType"].get<std::string>() == "MAX_POSITION") {
            mpf.maxPosition = std::stod(filter["maxPosition"].get<std::string>());
        }
    }

    return SymbolFilter(pf, lsf, mlsf, nf, mnf, mpf);
}
