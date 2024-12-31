#pragma once

#include <nlohmann/json.hpp>
#include <map>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <condition_variable>
#include <vector>
#include <set>
#include <stdexcept>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <fstream>
#include <sstream>

#include "common/WebSocketListener.h"
#include "bnb/utils/BNBRequests/Authentication.h"
#include "common/logger.hpp"
#include "fin/Order.h"
#include "bnb/marketConnection/BNBMarketConnectionConfig.h"
#include "bnb/utils/BNBRequests/RequestsBuilder.h"

class BNBBroker : public WebSocketListener {
public:
    explicit BNBBroker(const BNBMarketConnectionConfig& config);
    virtual ~BNBBroker();

    void start();
    void stop();

    std::string sendRequest(const std::string& requestId, const std::string& requestBody);
    nlohmann::json getResponseForId(const std::string& id);

protected:
    void onMessage(websocketpp::connection_hdl hdl, websocketpp::client<websocketpp::config::asio_client>::message_ptr msg) override;

private:
    std::string loadPrivateKey(const std::string& keyPath);

    std::string apiKey_;
    std::string secretKey_;
    std::string uri_;
    bool loginOnConnection_;
    std::string signMethod_;

    //Login utils
    std::string loginRequestId_;
    std::mutex login_mutex_;
    std::condition_variable login_cv_;
    bool is_logged_in_;

    std::map<std::string, nlohmann::json> stored_responses_;
    std::set<std::string> pending_requests_;
    std::mutex response_mutex_;
    std::condition_variable response_cv_;

    std::thread bws_thread_;
    std::atomic<bool> brunning_ = true;
};
