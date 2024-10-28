//Impelements the https://github.com/binance/binance-spot-api-docs/blob/master/filters.md
#ifndef SYMBOL_FILTER_H
#define SYMBOL_FILTER_H

#include <cmath>
#include <algorithm>
// Structs for various filters
struct PriceFilter {
    double minPrice;
    double maxPrice;
    double tickSize;
};

struct LotSizeFilter {
    double minQty;
    double maxQty;
    double stepSize;
};

struct MarketLotSizeFilter {
    double minQty;
    double maxQty;
    double stepSize;
};

struct NotionalFilter {
    double minNotional;
    bool applyMinToMarket;
    double maxNotional;
    bool applyMaxToMarket;
    int avgPriceMins;
};

struct MinNotionalFilter {
    double minNotional;
    bool applyToMarket;
    int avgPriceMins;
};

struct MaxPositionFilter {
    double maxPosition;
};

// SymbolFilter Class
class SymbolFilter {
private:
    PriceFilter priceFilter;
    LotSizeFilter lotSizeFilter;
    MarketLotSizeFilter marketLotSizeFilter;
    NotionalFilter notionalFilter;
    MinNotionalFilter minNotionalFilter;
    MaxPositionFilter maxPositionFilter;

    // Helper function to round values
    double roundToNearest(double value, double stepSize, bool roundUp);

public:
    // Constructor
    SymbolFilter(
        const PriceFilter& pf,
        const LotSizeFilter& lsf,
        const MarketLotSizeFilter& mlsf,
        const NotionalFilter& nf,
        const MinNotionalFilter& mnf,
        const MaxPositionFilter& mpf
    );

    // Methods
    double roundPrice(double price, bool roundUp=false);
    double roundQty(double qty, bool roundUp=false);

    bool validatePrice(double price);
    bool validateQuantity(double quantity);
    bool validateNotional(double price, double quantity);
    bool validateMaxPosition(double position);
};

#endif // SYMBOL_FILTER_H
