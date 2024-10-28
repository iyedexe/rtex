// KlineMDFrame.h
#ifndef KLINE_MDFRAME_H
#define KLINE_MDFRAME_H

#include "MarketDataFrame.h"

class KlineMDFrame : public MarketDataFrame {
public:
    std::string symbol;
    std::string open;
    std::string high;
    std::string low;
    std::string close;
    std::string volume;
    // Add other Kline attributes as needed
    std::string to_str() const override
    {
        return symbol+ ";" + std::to_string(timestamp.time_since_epoch().count()) + ";" + open + ";" + high + ";" + low + ";" + close + ";" + volume;
    }
    static std::string getHeader()
    {
        return "symbol;timestamp;open;high;low;close;volume";
    }  

};

#endif // KLINE_MDFRAME_H
