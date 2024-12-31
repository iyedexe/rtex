#include "bnb/utils/BNBRequests/RequestsBuilder.h"

namespace BNBRequests
{
    class General
    {
    public:
        static request ping();
        static request checkServerTime();
        static request exchangeInformation(std::vector<std::string> symbols);
    };
}