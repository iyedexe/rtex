#include "bnb/utils/BNBRequests/RequestsBuilder.h"

namespace BNBRequests
{
    class Authentication
    {
    public:
        static request logIn();
        static request querySessionStatus();
        static request logOut();
    };
}

