#include "bnb/marketConnection/BNBBroker.h"


BNBBroker::BNBBroker(const BNBMarketConnectionConfig& config): 
    apiKey_(config.apiKey), 
    uri_(config.apiWsEndpoint),
    loginOnConnection_(config.loginOnConnection),
    signMethod_(config.signMethod)
{
    if (signMethod_=="HMAC")
    {
        secretKey_ = config.apiSecret;
    }
    else if ((signMethod_=="ED25519") || (signMethod_=="RSA")) 
    {
        secretKey_ = loadPrivateKey(config.privateKeyPath);
    }
    else
    {
        throw std::runtime_error("[BNBBroker] Binance API sign method unsupported : <"+signMethod_+">.");
    }
    requests_ = std::make_unique<BNBRequests>(config.apiKey, secretKey_);

    LOG_INFO("[BNBBroker] BNBBroker initialized with API Endpoint: {}", uri_);
}

BNBBroker::~BNBBroker() {
    stop();
    LOG_INFO("BNBBroker destroyed.");
}

void BNBBroker::start() {
    LOG_INFO("[BNBBroker] Starting BNBBroker");
    bws_thread_ = std::thread([this]() {
        brunning_ = true;
        LOG_INFO("[BNBBroker] Connecting ..");
        connect(uri_);
        WebSocketListener::startClient();
        LOG_INFO("[BNBBroker] BNBBroker WebSocket connection lost");
    });
    if (loginOnConnection_)
    {
        LOG_INFO("[BNBBroker] Logging in ...");
        if (signMethod_ != "ED25519")
        {
            throw std::runtime_error("[BNBBroker] Unsupported login on connection with sign method : " + signMethod_);
        }
        loginRequestId_ = sendRequest(&BNBRequests::logIn);
    }
}

void BNBBroker::stop() {
    if (brunning_) {
        LOG_INFO("Stopping BNBBroker WebSocket connection...");
        stopClient();
        brunning_ = false;
        if (bws_thread_.joinable()) {
            bws_thread_.join();
            LOG_INFO("WebSocket connection thread joined.");
        }
    }
}

nlohmann::json BNBBroker::getResponseForId(const std::string& id) {
    std::unique_lock<std::mutex> lock(response_mutex_);

    if (pending_requests_.find(id) == pending_requests_.end()) {
        LOG_WARNING("Request ID not found: {}", id);
        throw std::runtime_error("Request ID not found or no request was made with this ID.");
    }

    LOG_INFO("Waiting for response for Request ID: {}", id);
    response_cv_.wait(lock, [this, &id]() {
        return stored_responses_.find(id) != stored_responses_.end();
    });

    LOG_INFO("Response received for Request ID: {}", id);
    return stored_responses_[id];
}

void BNBBroker::onMessage(websocketpp::connection_hdl hdl, websocketpp::client<websocketpp::config::asio_client>::message_ptr msg) {
    try
    {
        std::string payload = msg->get_payload();
        auto json_data = nlohmann::json::parse(payload);

        std::lock_guard<std::mutex> lock(response_mutex_);
        std::string id = json_data.value("id", "");
        if (!id.empty()) {
            bool isError=false;
            if (json_data.contains("error")) {
                int status = json_data.value("status", -1);
                int errorCode = json_data["error"].value("code", 0);
                std::string errorMsg = json_data["error"].value("msg", "Unknown error");
                isError=true;
                LOG_ERROR("[BROKER] Error received for message id {} : Status: {}, Code: {}, Message: {}", id, status, errorCode, errorMsg);
            }

            if (id == loginRequestId_)
            {
                if (isError)
                {
                    throw std::runtime_error("[BROKER] Authorization failed see error above");
                }
                std::unique_lock<std::mutex> lock(login_mutex_);
                if (json_data.contains("result")) {
                    std::string apiKey = json_data["result"].value("apiKey", "");
                    std::string authorizedSince = json_data["result"].value("authorizedSince", "");
                    if (apiKey.empty() || authorizedSince.empty())
                    {
                        throw std::runtime_error("BROKER] Authorization failed see error above, response apiKey: "+apiKey+", authorizedSince: "+authorizedSince);
                    }
                    login_cv_.notify_all();
                }
            } 

            stored_responses_[id] = std::move(json_data);
            pending_requests_.erase(id);

            LOG_INFO("[BROKER] Response stored for ID: {}", id);
            response_cv_.notify_all();
        } else {
            LOG_WARNING("[BROKER] Received a message without an ID. Message: {}", payload);
        }
    }
    catch(const std::exception& e)
    {
        LOG_ERROR("[BROKER] onMessage error: {}", e.what());
    }
}

std::string BNBBroker::loadPrivateKey(const std::string& keyPath) {
    std::ifstream file(keyPath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + keyPath);
    }

    std::ostringstream key_content;
    std::string line;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#' || line.find("----") != std::string::npos) {
            continue;
        }
        key_content << line << '\n';  // Append the valid lines to the result
    }

    return key_content.str();
}