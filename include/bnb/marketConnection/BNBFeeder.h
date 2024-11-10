#pragma once

#include <string>
#include <thread>
#include <atomic>
#include <set>

#include <nlohmann/json.hpp>
#include <fmt/ranges.h>

#include "bnb/marketData/BookTickerMDFrame.h"
#include "bnb/marketData/KlineMDFrame.h"
#include "bnb/marketData/AggTradeMDFrame.h"
#include "bnb/marketData/MarketDataFrame.h"
#include "bnb/marketConnection/BNBMarketConnectionConfig.h"
#include "common/WebSocketListener.h"


template <typename StreamType>
class BNBFeeder : public WebSocketListener {
public:
    BNBFeeder(const BNBMarketConnectionConfig& config);
    virtual ~BNBFeeder();

    int subscribeToTickers(const std::vector<std::string>& symbols);
    void start();
    void stop();
    StreamType getUpdate();

protected:
    void onMessage(websocketpp::connection_hdl hdl, websocketpp::client<websocketpp::config::asio_client>::message_ptr msg) override;
    void onClose(websocketpp::connection_hdl hdl);
    void onFail(websocketpp::connection_hdl hdl);

private:
    StreamType parseData(const nlohmann::json& json_data);
    std::string getStreamName();
    size_t maxStreamsSubs_;

    std::string uri_;
    std::thread fws_thread_;
    std::atomic<bool> frunning_;
    bool wsPersistConnection_;

    std::set<int> pending_requests_;
    int next_request_id_ = 1;
    std::vector<std::string> subscription_list_;

    std::queue<StreamType> update_queue_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;
};

template <>
BookTickerMDFrame BNBFeeder<BookTickerMDFrame>::parseData(const nlohmann::json& json_data);

template <>
KlineMDFrame BNBFeeder<KlineMDFrame>::parseData(const nlohmann::json& json_data);

template <>
AggTradeMDFrame BNBFeeder<AggTradeMDFrame>::parseData(const nlohmann::json& json_data);
