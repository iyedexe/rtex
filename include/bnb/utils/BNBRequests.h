#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <sodium.h>
#include <map>
#include "common/logger.hpp"
#include <fmt/ranges.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/evp.h>


using request = std::pair<std::string, std::string>;

class BNBRequests
{
public:
    BNBRequests(const std::string& apiKey, const std::string& secretKey);

    // session handling
    request logIn();
    request querySessionStatus();
    request logOut();

    // signed requests
    request placeOrder(const std::string& symbol, const std::string& side, double quantity, double price);
    request cancelAllOpenOrders(const std::string& symbol);
    request getAccountInformation();

    // unsigned
    request getExchangeInfo(const std::vector<std::string>& symbols);
    request SymbolOrderBookTicker(const std::vector<std::string>& symbols); 


private:
    std::string apiKey_;
    std::string secretKey_;
    bool loggedIn_{false};
    std::string generateED25519Signature(const std::string& secretKey, std::map<std::string, std::string>& params);
    void signRequestHMAC(std::map<std::string, std::string>& params);
    std::string getTimestamp();
    std::string generateRequestId();
};