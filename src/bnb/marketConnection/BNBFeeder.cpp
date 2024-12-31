#include "bnb/marketConnection/BNBFeeder.h"
#include "common/logger.hpp"

template <typename StreamType>
BNBFeeder<StreamType>::BNBFeeder(const BNBMarketConnectionConfig& config) : 
    frunning_(false), 
    uri_(config.streamsWsEndpoint), 
    maxStreamsSubs_(config.maxStreamsSubs), 
    wsPersistConnection_(config.wsPersistConnection) {
}

template <typename StreamType>
BNBFeeder<StreamType>::~BNBFeeder() {
    stop();
}

template <typename StreamType>
void BNBFeeder<StreamType>::start() {
    fws_thread_ = std::thread([this]() {
        frunning_= true;
        connect(uri_);
        WebSocketListener::startClient();
    });
}

template <typename StreamType>
void BNBFeeder<StreamType>::stop() {
    if (frunning_) {
        frunning_ = false;
        WebSocketListener::stopClient();
        if (fws_thread_.joinable()) {
            fws_thread_.join();
        }
    }
}

template <typename StreamType>
void BNBFeeder<StreamType>::onMessage(websocketpp::connection_hdl hdl, websocketpp::client<websocketpp::config::asio_client>::message_ptr msg) {
    try {
        std::string payload = msg->get_payload();
        LOG_DEBUG("[FEEDER] onMessage: {}", payload);

        auto json_data = nlohmann::json::parse(payload);

        // Check if the message contains an error object
        if (json_data.contains("error")) {
            auto error_data = json_data["error"];
            int error_code = error_data.value("code", 0);
            std::string error_msg = error_data.value("msg", "Unknown error");

            LOG_ERROR("[FEEDER] Error received - Code: {}, Message: {}", error_code, error_msg);

            // If the message has an ID, log that too
            if (json_data.contains("id")) {
                int message_id = json_data["id"];
                LOG_ERROR("[FEEDER] Error associated with message ID: {}", message_id);
            }

            return;  // Early return since this was an error message
        }

        // Check if the message is a response to a request
        if (json_data.contains("id")) {
            int message_id = json_data["id"];
            if (pending_requests_.count(message_id)) {
                pending_requests_.erase(message_id);
                LOG_INFO("[FEEDER] Subscription confirmed for message ID: {}", message_id);

                // Additional processing for the response if needed
                if (json_data.contains("result") && json_data["result"].is_null()) {
                    LOG_INFO("[FEEDER] Subscription to stream was successful.");
                } else {
                    LOG_WARNING("[FEEDER] Unexpected result in subscription response: {}", payload);
                }

                return;  // Early return since this was a response, not market data
            }
        }

        // If the message is not a response or an error, it's likely market data
        StreamType dataFrame = parseData(json_data);
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            update_queue_.push(dataFrame);
        }
        queue_cv_.notify_one();
    } catch (const std::exception& e) {
        LOG_ERROR("[FEEDER] onMessage error: {}", e.what());
    }
}

template <typename StreamType>
void BNBFeeder<StreamType>::onClose(websocketpp::connection_hdl hdl) {
    WebSocketListener::onClose(hdl);
    if (wsPersistConnection_)
    {
        stop();
        start();
        subscribeToTickers(subscription_list_);
    }
}

template <typename StreamType>
void BNBFeeder<StreamType>::onFail(websocketpp::connection_hdl hdl) {
    WebSocketListener::onFail(hdl);
    if (wsPersistConnection_)
    {
        stop();
        start();
        subscribeToTickers(subscription_list_);
    }
}


template <typename StreamType>
StreamType BNBFeeder<StreamType>::getUpdate() {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    queue_cv_.wait(lock, [this] { return !update_queue_.empty() || !frunning_; });

    if (!frunning_ && update_queue_.empty()) {
        throw std::runtime_error("No more updates, feeder stopped.");
    }

    StreamType dataFrame = update_queue_.front();
    update_queue_.pop();
    return dataFrame;
}

template <typename StreamType>
int BNBFeeder<StreamType>::subscribeToTickers(const std::vector<std::string>& symbols) {
    {
        std::unique_lock<std::mutex> lock(connection_mutex_);
        connection_cv_.wait(lock, [this] { return is_connected_; });
    }
    LOG_INFO("[FEEDER] Subscribing to {} tickers ", symbols.size());
    subscription_list_=symbols;

    std::string streamName = getStreamName();
    std::vector<std::string> streams;
    int currentSize=0;
    for (const auto& symbol : symbols) {
        std::string lowercase_symbol = symbol;
        std::transform(lowercase_symbol.begin(), lowercase_symbol.end(), lowercase_symbol.begin(), ::tolower);
        streams.push_back(lowercase_symbol + "@" + streamName);
        if(++currentSize==maxStreamsSubs_)
        {
            LOG_WARNING("[FEEDER] Subscripion size is higher than maximum allowed {} vs max {}", symbols.size(), maxStreamsSubs_);
            break;
        }
    }

    size_t chunk_size = maxStreamsSubs_/4;
    for (size_t i = 0; i < streams.size(); i += chunk_size) {
        std::vector<std::string> chunk(streams.begin() + i, streams.begin() + std::min(streams.size(), i + chunk_size));

        int request_id = next_request_id_++;
        nlohmann::json request;
        request["method"] = "SUBSCRIBE";
        request["params"] = chunk;
        request["id"] = request_id;

        std::string request_str = request.dump();
        writeWS(request_str);
        pending_requests_.insert(request_id);

//        LOG_INFO("Subscription request sent for symbols (chunk {} - {}): {} with request ID: {}",
  //                   i + 1, std::min(i + chunk_size, streams.size()), fmt::join(chunk, ", "), request_id);
    }
    return currentSize;
}


template <typename StreamType>
std::string BNBFeeder<StreamType>::getStreamName() {
    if constexpr (std::is_same_v<StreamType, BookTickerMDFrame>) {
        return "bookTicker";
    } else if constexpr (std::is_same_v<StreamType, KlineMDFrame>) {
        return "kline_1m";  // Assuming 1-minute kline stream, adjust as needed
    } else if constexpr (std::is_same_v<StreamType, AggTradeMDFrame>) {
        return "aggTrade";
    } else {
        throw std::runtime_error("Unknown stream type");
    }
}

// Template specializations
template <>
BookTickerMDFrame BNBFeeder<BookTickerMDFrame>::parseData(const nlohmann::json& json_data) {
    BookTickerMDFrame dataFrame;
    dataFrame.symbol = json_data["s"];
    dataFrame.bestBidPrice = std::stod(json_data["b"].get<std::string>());
    dataFrame.bestBidQty = std::stod(json_data["B"].get<std::string>());
    dataFrame.bestAskPrice = std::stod(json_data["a"].get<std::string>());
    dataFrame.bestAskQty = std::stod(json_data["A"].get<std::string>());
    return dataFrame;
}

template <>
KlineMDFrame BNBFeeder<KlineMDFrame>::parseData(const nlohmann::json& json_data) {
    KlineMDFrame dataFrame;
    dataFrame.symbol = json_data["s"];
    dataFrame.open = json_data["o"];
    dataFrame.high = json_data["h"];
    dataFrame.low = json_data["l"];
    dataFrame.close = json_data["c"];
    dataFrame.volume = json_data["v"];
    return dataFrame;
}

template <>
AggTradeMDFrame BNBFeeder<AggTradeMDFrame>::parseData(const nlohmann::json& json_data) {
    AggTradeMDFrame dataFrame;
    dataFrame.symbol = json_data["s"];
    dataFrame.price = json_data["p"];
    dataFrame.quantity = json_data["q"];
    dataFrame.tradeId = json_data["a"];
    return dataFrame;
}

// Explicit template instantiations to avoid linker errors
template class BNBFeeder<BookTickerMDFrame>;
template class BNBFeeder<KlineMDFrame>;
template class BNBFeeder<AggTradeMDFrame>;
