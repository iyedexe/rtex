#include "bnb/utils/SymbolFilter.h"

// Helper function to round values
double SymbolFilter::roundToNearest(double value, double stepSize, bool roundUp) {
    double remainder = fmod(value, stepSize);
    if (remainder == 0) return value;
    return roundUp ? (value - remainder + stepSize) : (value - remainder);
}

// Constructor
SymbolFilter::SymbolFilter(
    const PriceFilter& pf,
    const LotSizeFilter& lsf,
    const MarketLotSizeFilter& mlsf,
    const NotionalFilter& nf,
    const MinNotionalFilter& mnf,
    const MaxPositionFilter& mpf
) : priceFilter(pf), lotSizeFilter(lsf), marketLotSizeFilter(mlsf), 
    notionalFilter(nf), minNotionalFilter(mnf), maxPositionFilter(mpf) {}

// Method to round the price using PRICE_FILTER
double SymbolFilter::roundPrice(double price, bool roundUp) {
    if (priceFilter.tickSize > 0) {
        price = roundToNearest(price, priceFilter.tickSize, roundUp);
    }
    if (priceFilter.minPrice > 0)
    {
        price = std::max(price, priceFilter.minPrice);
    }
    if (priceFilter.maxPrice > 0)
    {
        price = std::min(price, priceFilter.maxPrice);
    }

    return price;
}

// Method to round the quantity using both LOT_SIZE and MARKET_LOT_SIZE filters
double SymbolFilter::roundQty(double qty, bool roundUp) {
    double roundedQty = qty;

    // Apply MARKET_LOT_SIZE rounding
    if (marketLotSizeFilter.stepSize > 0) {
        roundedQty = roundToNearest(roundedQty, marketLotSizeFilter.stepSize, roundUp);
    }
    if (marketLotSizeFilter.minQty > 0) {
        roundedQty = std::max(roundedQty, marketLotSizeFilter.minQty);
    }
    if (marketLotSizeFilter.maxQty > 0) {
        roundedQty = std::min(roundedQty, marketLotSizeFilter.maxQty);
    }

    // Apply LOT_SIZE rounding
    if (lotSizeFilter.stepSize > 0) {
        roundedQty = roundToNearest(roundedQty, lotSizeFilter.stepSize, roundUp);
    }
    if (lotSizeFilter.minQty > 0) {
        roundedQty = std::max(roundedQty, lotSizeFilter.minQty);
    }
    if (lotSizeFilter.maxQty > 0) {
        roundedQty = std::min(roundedQty, lotSizeFilter.maxQty);
    }

    return roundedQty;
}

// Validate price using PRICE_FILTER
bool SymbolFilter::validatePrice(double price) {
    return (price >= priceFilter.minPrice && price <= priceFilter.maxPrice &&
            fmod(price, priceFilter.tickSize) == 0);
}

// Validate quantity using both LOT_SIZE and MARKET_LOT_SIZE filters
bool SymbolFilter::validateQuantity(double quantity) {
    bool validLotSize = (quantity >= lotSizeFilter.minQty && quantity <= lotSizeFilter.maxQty &&
                         fmod(quantity, lotSizeFilter.stepSize) == 0);

    bool validMarketLotSize = (quantity >= marketLotSizeFilter.minQty && quantity <= marketLotSizeFilter.maxQty &&
                               fmod(quantity, marketLotSizeFilter.stepSize) == 0);

    return validLotSize && validMarketLotSize;
}

// Validate notional using NOTIONAL and MIN_NOTIONAL filters
bool SymbolFilter::validateNotional(double price, double quantity) {
    double notional = price * quantity;
    bool validNotional = (notional >= notionalFilter.minNotional && notional <= notionalFilter.maxNotional);
    bool validMinNotional = (notional >= minNotionalFilter.minNotional);

    return validNotional && validMinNotional;
}

// Validate max position using MAX_POSITION filter
bool SymbolFilter::validateMaxPosition(double position) {
    return position <= maxPositionFilter.maxPosition;
}
