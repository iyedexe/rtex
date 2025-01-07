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
    RequestsBuilder::getInstance(apiKey_, secretKey_);
    LOG_INFO("[BNBBroker] BNBBroker initialized with API Endpoint: {}, sign method used : {}", uri_, signMethod_);
}

BNBBroker::~BNBBroker() {
    stop();
    LOG_INFO("[BNBBroker] BNBBroker destroyed.");
}

void BNBBroker::start() {
    bws_thread_ = std::thread([this]() {
        brunning_ = true;
        connect(uri_);
        WebSocketListener::startClient();
    });
    if (loginOnConnection_)
    {
        LOG_INFO("[BNBBroker] Logging in ...");
        if (signMethod_ != "ED25519")
        {
            throw std::runtime_error("[BNBBroker] Unsupported login on connection with sign method : " + signMethod_);
        }
        auto req = BNBRequests::Authentication::logIn();
        loginRequestId_ = sendRequest(req.first, req.second);
    }
}

void BNBBroker::stop() {
    if (brunning_) {
        LOG_INFO("[BNBBroker] Stopping BNBBroker WebSocket connection...");
        stopClient();
        brunning_ = false;
        if (bws_thread_.joinable()) {
            bws_thread_.join();
            LOG_INFO("[BNBBroker] WebSocket connection thread joined.");
        }
    }
}

std::string BNBBroker::sendRequest(const std::string& requestId, const std::string& requestBody)
{
    LOG_INFO("[BNBBroker] Sending request");
    // if (loginOnConnection_ && !(std::is_same<decltype(func), decltype(&BNBRequests::logIn)>::value))
    // {
    //     LOG_INFO("[BNBBroker] Waiting for login");
    //     std::unique_lock<std::mutex> lock(login_mutex_);
    //     login_cv_.wait(lock, [this] { return is_logged_in_; });
    // }

    {
        std::lock_guard<std::mutex> lock(response_mutex_);
        pending_requests_.insert(requestId);
    }

    writeWS(requestBody);
    LOG_INFO("[BNBBroker] Request sent, ID: {}", requestId);
    return requestId;
}

nlohmann::json BNBBroker::getResponseForId(const std::string& id) {
    std::unique_lock<std::mutex> lock(response_mutex_);

    if (pending_requests_.find(id) == pending_requests_.end()) {
        LOG_WARNING("[BNBBroker] Request ID not found: {}", id);
        throw std::runtime_error("Request ID not found or no request was made with this ID.");
    }

    LOG_INFO("[BNBBroker] Waiting for response for Request ID: {}", id);
    response_cv_.wait(lock, [this, &id]() {
        return stored_responses_.find(id) != stored_responses_.end();
    });

    LOG_INFO("[BNBBroker] Response received for Request ID: {}", id);
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
                LOG_ERROR("[BNBBroker] Error received for message id {} : Status: {}, Code: {}, Message: {}", id, status, errorCode, errorMsg);
            }

            if (id == loginRequestId_)
            {
                if (isError)
                {
                    throw std::runtime_error("[BNBBroker] Authorization failed see error above");
                }
                std::unique_lock<std::mutex> lock(login_mutex_);
                if (json_data.contains("result")) {
                    std::string apiKey = json_data["result"].value("apiKey", "");
                    std::string authorizedSince = json_data["result"].value("authorizedSince", "");
                    if (apiKey.empty() || authorizedSince.empty())
                    {
                        throw std::runtime_error("[BNBBroker] Authorization failed see error above, response apiKey: "+apiKey+", authorizedSince: "+authorizedSince);
                    }
                    login_cv_.notify_all();
                }
            } 

            stored_responses_[id] = std::move(json_data);
            pending_requests_.erase(id);

            LOG_INFO("[BNBBroker] Response stored for ID: {}", id);
            response_cv_.notify_all();
        } else {
            LOG_WARNING("[BNBBroker] Received a message without an ID. Message: {}", payload);
        }
    }
    catch(const std::exception& e)
    {
        LOG_ERROR("[BNBBroker] onMessage error: {}", e.what());
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