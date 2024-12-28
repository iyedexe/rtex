#pragma once
#include "common/WebSocketListener.h"
#include "bnb/utils/BNBRequests.h"
#include "common/logger.hpp"
#include "fin/Order.h"
#include "bnb/marketConnection/BNBMarketConnectionConfig.h"
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
#include <type_traits> 
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <stdexcept>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <fstream>
#include <sstream>

class BNBBroker : public WebSocketListener {
public:
    explicit BNBBroker(const BNBMarketConnectionConfig& config);
    virtual ~BNBBroker();

    void start();
    void stop();

    template<typename T, typename... Args>
    std::string sendRequest(request (T::*func)(Args...), Args&&... args)
    {
        LOG_INFO("[BNBBroker] Sending request");
        // Wait until connected
        {
            std::unique_lock<std::mutex> lock(connection_mutex_);
            connection_cv_.wait(lock, [this] { return is_connected_; });
        }
        if (loginOnConnection_ && !(std::is_same<decltype(func), decltype(&BNBRequests::logIn)>::value))
        {
            LOG_INFO("[BNBBroker] Waiting for login");
            std::unique_lock<std::mutex> lock(login_mutex_);
            login_cv_.wait(lock, [this] { return is_logged_in_; });
        }

        request req = ((*requests_).*func)(std::forward<Args>(args)...);
        {
            std::lock_guard<std::mutex> lock(response_mutex_);
            pending_requests_.insert(req.first);
        }

        send(req.second);
        LOG_INFO("[BNBBroker] Request sent, ID: {}", req.first);
        return req.first;
    }
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

    std::unique_ptr<BNBRequests> requests_;
};
