#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include <sodium.h>
#include <map>
#include "common/logger.hpp"
#include "fin/Order.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include "bnb/utils/BNBRequests/RequestsHelper.h"

using request = std::pair<std::string, std::string>;

class RequestsBuilder
{
public:
    static request basicRequest(const std::string& method);
    static request paramsUnsignedRequest(const std::string& method, const nlohmann::json& params);
    static request paramsSignedRequest(const std::string& method, std::map<std::string, std::string>& params);

private:
    inline static RequestsBuilder* instance = nullptr;
    RequestsBuilder(const std::string& apiKey, const std::string& secretKey)
     : apiKey_(apiKey), secretKey_(secretKey) {}

    std::string apiKey_;
    std::string secretKey_;
    bool loggedIn_{false};

public:
    RequestsBuilder(const RequestsBuilder&) = delete;
    RequestsBuilder& operator=(const RequestsBuilder&) = delete;

    // Static method to get the singleton instance
    static RequestsBuilder* getInstance(const std::string& apiKey, const std::string& secretKey) {
        instance = new RequestsBuilder(apiKey, secretKey);
        return instance;
    }
};
