#include "bnb/utils/BNBRequests/Trading.h"

namespace BNBRequests
{
    request Trading::placeNewOrder(const std::string& symbol, Way side, OrderType type, double quantity, double price){
        std::map<std::string, std::string> params{
                {"symbol", "symbol"},
                {"side", (side==Way::BUY) ? "BUY" : "SELL"},
                {"type", (type==OrderType::MARKET) ? "MARKET" : "LIMIT"},
                {"quantity", std::to_string(quantity)}
        };
        if (price > 0)
        {
            params["price"] = std::to_string(price);
        }
      
        return RequestsBuilder::paramsSignedRequest("order.place", params);    
 
    }

    request Trading::testNewOrder(const std::string& symbol, Way side, OrderType type, double quantity, double price){
        std::map<std::string, std::string> params{
                {"symbol", symbol},
                {"side", (side==Way::BUY) ? "BUY" : "SELL"},
                {"type", (type==OrderType::MARKET) ? "MARKET" : "LIMIT"},
                {"quantity", std::to_string(quantity)},
                {"computeCommissionRates", "true"}
        };
        if (price > 0)
        {
            params["price"] = std::to_string(price);
        }
      
        return RequestsBuilder::paramsSignedRequest("order.test", params);    
 
    }

    request Trading::cancelOrders(){
        throw std::runtime_error("[BNBREQUESTS] Not yet implemented");
        return std::make_pair("", "");
    }
}

