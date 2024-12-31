#include "bnb/utils/BNBRequests/RequestsBuilder.h"

namespace BNBRequests
{
    class Account
    {
    public:
        static request information();
        static request unfilledOrderCount();
        static request orderHistory();
        static request allOrderHistory();
        static request tradeHistory();
        static request preventedMatches();
        static request allocations();
        static request commissionRates();
    };
}

