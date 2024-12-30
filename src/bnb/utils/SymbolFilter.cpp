#include "bnb/utils/SymbolFilter.h"
#include <algorithm>
#include <limits>
#include "common/logger.hpp"

double SymbolFilter::roundToNearest(double value, double stepSize, bool roundUp) {
    if (stepSize == 0) return value;
    double remainder = fmod(value, stepSize);
    if (remainder == 0) return value;
    return roundUp ? (value - remainder + stepSize) : (value - remainder);
}

SymbolFilter::SymbolFilter(
    const PriceFilter& pf,
    const LotSizeFilter& lsf,
    const MarketLotSizeFilter& mlsf,
    const NotionalFilter& nf,
    const MinNotionalFilter& mnf,
    const MaxPositionFilter& mpf
) : priceFilter(pf), lotSizeFilter(lsf), marketLotSizeFilter(mlsf), 
    notionalFilter(nf), minNotionalFilter(mnf), maxPositionFilter(mpf) {}


double SymbolFilter::roundPrice(double price, bool roundUp) {

    double adjustedPrice = std::clamp(
        roundToNearest(price, priceFilter.tickSize, roundUp), 
        priceFilter.minPrice, 
        (priceFilter.maxPrice!=0) ? priceFilter.maxPrice : std::numeric_limits<double>::max()
    );

    return adjustedPrice;
}

double SymbolFilter::roundQty(double qty, bool roundUp) {
    double minQty_=std::max(marketLotSizeFilter.minQty, lotSizeFilter.minQty);
    double maxQty_=std::min(
        (marketLotSizeFilter.maxQty==0) ? std::numeric_limits<double>::max() : marketLotSizeFilter.maxQty,
        (lotSizeFilter.maxQty==0) ? std::numeric_limits<double>::max() : lotSizeFilter.maxQty
    );
    double lotSize_=std::max(marketLotSizeFilter.stepSize, lotSizeFilter.stepSize);

    LOG_DEBUG("Rounding Quantity, min={}, max={}, lot_size={}", minQty_ , maxQty_, lotSize_);
    double roundedQty = std::clamp(
        roundToNearest(qty, lotSize_, roundUp), 
        0.0, 
        (maxQty_==0) ? std::numeric_limits<double>::max() : maxQty_
    );

    return roundedQty;
}

bool SymbolFilter::validatePrice(double price) {
    return (price == roundPrice(price));
}

bool SymbolFilter::validateQuantity(double quantity) {
    return (quantity == roundQty(quantity));
}

bool SymbolFilter::validateNotional(double price, double quantity) {
    double notional = price * quantity;
    bool validNotional = (notional >= notionalFilter.minNotional && notional <= notionalFilter.maxNotional);
    bool validMinNotional = (notional >= minNotionalFilter.minNotional);

    return validNotional && validMinNotional;
}

bool SymbolFilter::validateMaxPosition(double position) {
    return position <= maxPositionFilter.maxPosition;
}
