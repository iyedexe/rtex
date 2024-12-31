#include "bnb/utils/BNBRequests/RequestsBuilder.h"

namespace BNBRequests
{
    class Trading
    {
    public:
        static request placeNewOrder(const std::string& symbol, Way side, OrderType type, double quantity, double price=0);
        static request testNewOrder(const std::string& symbol, Way side, OrderType type, double quantity, double price=0);
        static request cancelOrders();
    };
}