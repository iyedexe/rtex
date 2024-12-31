#include "bnb/utils/BNBRequests/MarketData.h"


namespace BNBRequests
{
    request MarketData::orderBook(){
        throw std::runtime_error("[BNBREQUESTS] Not yet implemented");
        return std::make_pair("", "");
    }

    request MarketData::recentTrades(){
        throw std::runtime_error("[BNBREQUESTS] Not yet implemented");
        return std::make_pair("", "");
    }

    request MarketData::historicalTrades(){
        throw std::runtime_error("[BNBREQUESTS] Not yet implemented");
        return std::make_pair("", "");
    }

    request MarketData::aggregateTrades(){
        throw std::runtime_error("[BNBREQUESTS] Not yet implemented");
        return std::make_pair("", "");
    }

    request MarketData::klines(){
        throw std::runtime_error("[BNBREQUESTS] Not yet implemented");
        return std::make_pair("", "");
    }

    request MarketData::uiKlines(){
        throw std::runtime_error("[BNBREQUESTS] Not yet implemented");
        return std::make_pair("", "");
    }

    request MarketData::currentAvgPrice(){
        throw std::runtime_error("[BNBREQUESTS] Not yet implemented");
        return std::make_pair("", "");
    }

    request MarketData::tickerPriceChangeStats24h(){
        throw std::runtime_error("[BNBREQUESTS] Not yet implemented");
        return std::make_pair("", "");
    }

    request MarketData::tickerPriceStatsDay(){
        throw std::runtime_error("[BNBREQUESTS] Not yet implemented");
        return std::make_pair("", "");
    }

    request MarketData::tickerPriceChangeStatsCustom(){
        throw std::runtime_error("[BNBREQUESTS] Not yet implemented");
        return std::make_pair("", "");
    }

    request MarketData::symbolPriceTicker(){
        throw std::runtime_error("[BNBREQUESTS] Not yet implemented");
        return std::make_pair("", "");
    }

    request MarketData::symbolOrderBookTicker(const std::vector<std::string>& symbols){
        nlohmann::json params = nlohmann::json::object();
        if (!symbols.empty()) {
            params["symbols"] = symbols;
        }
        std::string method = "ticker.book";

        return RequestsBuilder::paramsUnsignedRequest(method, params);

    }
}