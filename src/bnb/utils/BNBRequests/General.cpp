#include "bnb/utils/BNBRequests/General.h"

namespace BNBRequests
{
    request General::ping()
    {
        return RequestsBuilder::basicRequest("ping");
    }

    request General::checkServerTime()
    {
        return RequestsBuilder::basicRequest("time");

    }

    request General::exchangeInformation(std::vector<std::string> symbols)
    {
        nlohmann::json params = nlohmann::json::object();
        if (!symbols.empty()) {
            params["symbols"] = symbols;
        }
        return RequestsBuilder::paramsUnsignedRequest("exchangeInfo", params);
    }
}
