#include "bnb/utils/BNBRequests/RequestsBuilder.h"

namespace BNBRequests
{
    class MarketData
    {
    public:
        static request orderBook();
        static request recentTrades();
        static request historicalTrades();
        static request aggregateTrades();
        static request klines();
        static request uiKlines();
        static request currentAvgPrice();
        static request tickerPriceChangeStats24h();
        static request tickerPriceStatsDay();
        static request tickerPriceChangeStatsCustom();
        static request symbolPriceTicker();
        static request symbolOrderBookTicker(const std::vector<std::string>& symbols);
    };
}