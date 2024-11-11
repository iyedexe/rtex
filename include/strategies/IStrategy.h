#pragma once
#include <string>
#include "fin/Signal.h"
#include "bnb/marketData/BookTickerMDFrame.h"

// Interface for all trading strategies
class IStrategy {
public:
    virtual ~IStrategy() = default;

    // Method to handle new incoming market data
    virtual std::optional<Signal> onMarketData(const BookTickerMDFrame& data) = 0;

    // Method to initialize any necessary resources or data
    virtual void initialize() = 0;

    // Method to clean up resources if needed
    virtual void shutdown() = 0;

    virtual void run() = 0;
};
