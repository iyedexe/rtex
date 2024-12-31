#include "bnb/utils/BNBRequests/Account.h"

namespace BNBRequests
{
    request Account::information(){
        std::map<std::string, std::string> params{
                {"omitZeroBalances", "true"}
        };
        std::string method = "account.status";
      
        return RequestsBuilder::paramsSignedRequest(method, params);
    }
    request Account::unfilledOrderCount(){
        throw std::runtime_error("[BNBREQUESTS] Not yet implemented");
        return std::make_pair("", "");
    }
    request Account::orderHistory(){
        throw std::runtime_error("[BNBREQUESTS] Not yet implemented");
        return std::make_pair("", "");
    }
    request Account::allOrderHistory(){
        throw std::runtime_error("[BNBREQUESTS] Not yet implemented");
        return std::make_pair("", "");
    }
    request Account::tradeHistory(){
        throw std::runtime_error("[BNBREQUESTS] Not yet implemented");
        return std::make_pair("", "");
    }
    request Account::preventedMatches(){
        throw std::runtime_error("[BNBREQUESTS] Not yet implemented");
        return std::make_pair("", "");
    }
    request Account::allocations(){
        throw std::runtime_error("[BNBREQUESTS] Not yet implemented");
        return std::make_pair("", "");
    }
    request Account::commissionRates(){
        throw std::runtime_error("[BNBREQUESTS] Not yet implemented");
        return std::make_pair("", "");
    }
}

