// BNBMarketConnectionConfig.h
#ifndef BNB_MARKET_CONNECTION_CONFIG_H
#define BNB_MARKET_CONNECTION_CONFIG_H

#include <string>

struct BNBMarketConnectionConfig {
    std::string streamsWsEndpoint;
    std::string apiWsEndpoint;
    std::string apiKey;
    std::string apiSecret;
    std::string privateKeyPath;
    size_t maxStreamsSubs;
    bool wsPersistConnection;
    bool loginOnConnection;
    std::string signMethod;
};

BNBMarketConnectionConfig loadConfig(const std::string& configFile);

#endif // BNB_MARKET_CONNECTION_CONFIG_H
