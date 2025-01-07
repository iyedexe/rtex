#include <set>
#include <nlohmann/json.hpp>

#include "common/WebSocketListener.h"

namespace WS
{
    namespace BNB
    {
        using RequestId = int;
        class StreamsClient: public WebSocketListener
        {
        public:
            StreamsClient();
            virtual ~StreamsClient();

            // response is request IDs            
            std::vector<RequestId> subscribeToStreams(const std::vector<std::string>& streamsNames);
            std::vector<RequestId> unSubscribeToStreams(const std::vector<std::string>& streamsNames);
            RequestId listSubscriptions();
            RequestId setProperty(const std::string& property, const std::string& value);
        
        protected:
            void onMessage(websocketpp::connection_hdl hdl, websocketpp::client<websocketpp::config::asio_client>::message_ptr msg) override;
            void onClose(websocketpp::connection_hdl hdl);
            void onFail(websocketpp::connection_hdl hdl);

        private:
            std::vector<RequestId> sendChunckedRequest(const std::string& method, const std::vector<std::string>& streamsNames);

            std::set<RequestId> pendingRequests_;
            int nextRequestId_ = 1;
            size_t maxStreamsSubs_;
        };

    }
}
