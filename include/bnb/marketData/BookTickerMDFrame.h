// BookTickerMDFrame.h
#ifndef BOOKTICKER_MDFRAME_H
#define BOOKTICKER_MDFRAME_H

#include "MarketDataFrame.h"

class BookTickerMDFrame : public MarketDataFrame {
public:
    std::string symbol;
    double bestBidPrice;
    double bestBidQty;
    double bestAskPrice;
    double bestAskQty;

    std::string to_str() const override
    {
        return symbol+ ";" + std::to_string(timestamp.time_since_epoch().count())+ ";" + std::to_string(bestBidPrice) + ";" + std::to_string(bestBidQty) + ";" + std::to_string(bestAskPrice) + ";" + std::to_string(bestAskQty);
    }
    static std::string getHeader()
    {
        return "symbol;timestamp;bestBidPrice;bestBidQty;bestAskPrice;bestAskQty";
    }  
};

#endif // BOOKTICKER_MDFRAME_H
