#ifndef LIVETRADER_H
#define LIVETRADER_H

#include "strategies/IStrategy.h"
#include "fin/Signal.h"
#include "utils/ExchangeInfo.h"
#include "bnb/marketConnection/BNBBroker.h"
#include "bnb/marketConnection/BNBFeeder.h"
#include "bnb/marketData/BookTickerMDFrame.h"
#include "strategies/CircularArb.h"
#include <string>
#include <optional>

class LiveTrader{
public:
    LiveTrader();
    void run();

private:
    BNBFeeder<BookTickerMDFrame> feeder_;
    BNBBroker broker_;
};

#endif // LIVETRADER_H

