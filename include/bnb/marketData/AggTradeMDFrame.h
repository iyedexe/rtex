// AggTradeMDFrame.h
#ifndef AGGTRADE_MDFRAME_H
#define AGGTRADE_MDFRAME_H

#include "MarketDataFrame.h"

class AggTradeMDFrame : public MarketDataFrame {
public:
    std::string symbol;
    std::string price;
    std::string quantity;
    std::string tradeId;
    // Add other AggTrade attributes as needed
    std::string to_str() const override
    {
        return symbol+ ";" + std::to_string(timestamp.time_since_epoch().count())+ ";" + price + ";" + quantity + ";" + tradeId;
    }
    static std::string getHeader()
    {
        return "symbol;timestamp;price;quantity;tradeId";
    } 
};

#endif // AGGTRADE_MDFRAME_H
