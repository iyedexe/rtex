// BNBMarketConnectionConfig.cpp
#include "bnb/marketConnection/BNBMarketConnectionConfig.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/lexical_cast.hpp>
#include <stdexcept>

BNBMarketConnectionConfig loadConfig(const std::string& configFile) {
    BNBMarketConnectionConfig config;
    boost::property_tree::ptree pt;

    try {
        boost::property_tree::ini_parser::read_ini(configFile, pt);

        config.streamsWsEndpoint = pt.get<std::string>("BNB_MARKET_CONNECTION.streams_ws_endpoint");
        config.apiWsEndpoint = pt.get<std::string>("BNB_MARKET_CONNECTION.api_ws_endpoint");
        config.apiKey = pt.get<std::string>("BNB_MARKET_CONNECTION.api_key");

        config.signMethod = boost::lexical_cast<std::string>(pt.get("BNB_MARKET_CONNECTION.sign_method", "HMAC"));
        if (config.signMethod == "HMAC")
        {
            config.apiSecret = pt.get<std::string>("BNB_MARKET_CONNECTION.api_secret");
        } 
        else
        {
            config.privateKeyPath = pt.get<std::string>("BNB_MARKET_CONNECTION.private_key_path");
        }
        config.maxStreamsSubs = boost::lexical_cast<size_t>(pt.get("BNB_MARKET_CONNECTION.maximum_streams_subscriptions", 200));
        config.wsPersistConnection = boost::lexical_cast<bool>(pt.get("BNB_MARKET_CONNECTION.ws_persist_connection", false));
        config.loginOnConnection = boost::lexical_cast<bool>(pt.get("BNB_MARKET_CONNECTION.login_on_connection", false));

    } catch (const boost::property_tree::ini_parser_error& e) {
        throw std::runtime_error("Failed to load config file: " + std::string(e.what()));
    } catch (const boost::property_tree::ptree_bad_path& e) {
        throw std::runtime_error("Missing parameter in config file: " + std::string(e.what()));
    }

    return config;
}
