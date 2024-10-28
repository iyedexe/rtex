// MarketDataFrame.h
#ifndef MARKETDATAFRAME_H
#define MARKETDATAFRAME_H

#include <string>
#include <chrono>

class MarketDataFrame {
public:
    MarketDataFrame() : timestamp(std::chrono::system_clock::now()) {}
    virtual ~MarketDataFrame() = default;
    virtual std::string to_str() const = 0;  // Pure virtual method to be implemented by subtypes
    static std::string getHeader();
    std::chrono::system_clock::time_point timestamp;
};

#endif // MARKETDATAFRAME_H
