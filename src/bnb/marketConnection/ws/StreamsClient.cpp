#include "bnb/marketConnection/ws/StreamsClient.hpp"
#include "common/logger.hpp"

namespace WS
{
    namespace BNB
    {
        // frunning_(false), 
        // uri_(config.streamsWsEndpoint), 
        // maxStreamsSubs_(config.maxStreamsSubs), 
        // wsPersistConnection_(config.wsPersistConnection) {
        std::vector<RequestId> StreamsClient::sendChunckedRequest(const std::string& method, const std::vector<std::string>& streamsNames)
        {
            std::vector<RequestId> subscribedStreamsId;
            size_t chunk_size = maxStreamsSubs_/4;
            for (size_t i = 0; i < streamsNames.size(); i += chunk_size) {
                std::vector<std::string> chunk(streamsNames.begin() + i, streamsNames.begin() + std::min(streamsNames.size(), i + chunk_size));

                int request_id = nextRequestId_++;
                nlohmann::json request;
                request["method"] = method;
                request["params"] = chunk;
                request["id"] = request_id;

                writeWS(request.dump());
                pendingRequests_.insert(request_id);
                subscribedStreamsId.push_back(request_id);
            }
            return subscribedStreamsId;
        }
        std::vector<RequestId> StreamsClient::subscribeToStreams(const std::vector<std::string>& streamsNames)
        {
            return sendChunckedRequest("SUBSCRIBE", streamsNames);
        }

        std::vector<RequestId> StreamsClient::unSubscribeToStreams(const std::vector<std::string>& streamsNames)
        {
            return sendChunckedRequest("UNSUBSCRIBE", streamsNames);

        }
        
        RequestId StreamsClient::listSubscriptions()
        {
            int request_id = nextRequestId_++;
            nlohmann::json request;
            request["method"] = "LIST_SUBSCRIPTIONS";
            request["id"] = request_id;

            writeWS(request.dump());
            pendingRequests_.insert(request_id);
        }

        RequestId StreamsClient::setProperty(const std::string& property, const std::string& value)
        {
            int request_id = nextRequestId_++;
            nlohmann::json request;
            request["method"] = "SET_PROPERTY";
            request["id"] = request_id;
            request["params"] = {property, value == "true" ? true: false};
            writeWS(request.dump());
            pendingRequests_.insert(request_id);
        }
        
        void StreamsClient::onMessage(websocketpp::connection_hdl hdl, websocketpp::client<websocketpp::config::asio_client>::message_ptr msg)
        {
            try {
                std::string payload = msg->get_payload();
                LOG_DEBUG("[STREAMS_CLIENT] onMessage: {}", payload);

                auto jsonData = nlohmann::json::parse(payload);

                // if response to a request
                if (jsonData.contains("id")) {
                    int messageId = jsonData["id"];

                    if (pendingRequests_.count(messageId)) {
                        pendingRequests_.erase(messageId);
                        LOG_INFO("[STREAMS_CLIENT] Response received for request ID: {}", messageId);

                        if (jsonData.contains("error")) {
                            auto error_data = jsonData["error"];
                            int error_code = error_data.value("code", 0);
                            std::string error_msg = error_data.value("msg", "Unknown error");

                            LOG_ERROR("[STREAMS_CLIENT] Error received - Code: {}, Message: {}", error_code, error_msg);
                        }

                        // // Additional processing for the response if needed
                        // if (json_data.contains("result") && json_data["result"].is_null()) {
                        //     LOG_INFO("[FEEDER] Subscription to stream was successful.");
                        // } else {
                        //     LOG_WARNING("[FEEDER] Unexpected result in subscription response: {}", payload);
                        // }

                        return;
                    }
                }

                // // If the message is not a response or an error, it's likely market data
                // StreamType dataFrame = parseData(json_data);
                // {
                //     std::lock_guard<std::mutex> lock(queue_mutex_);
                //     update_queue_.push(dataFrame);
                // }
                // queue_cv_.notify_one();
            } catch (const std::exception& e) {
                LOG_ERROR("[FEEDER] onMessage error: {}", e.what());
            }            
        }
        
        void StreamsClient::onClose(websocketpp::connection_hdl hdl)
        {
            WebSocketListener::onClose(hdl);
            // if (wsPersistConnection_)
            // {
            //     stop();
            //     start();
            //     subscribeToTickers(subscription_list_);
            // }
        }
        void StreamsClient::onFail(websocketpp::connection_hdl hdl)
        {
            WebSocketListener::onFail(hdl);
            // if (wsPersistConnection_)
            // {
            //     stop();
            //     start();
            //     subscribeToTickers(subscription_list_);
            // }
        }
    }
}