#include "bnb/utils/BNBRequests/RequestsHelper.h"

std::string RequestsHelper::generateED25519Signature(const std::string& secretKey, std::map<std::string, std::string>& params){

}
void RequestsHelper::signRequestHMAC(std::map<std::string, std::string>& params, const std::string& apiKey ,const std::string& secretKey){
    // https://developers.binance.com/docs/binance-spot-api-docs/web-socket-api/public-websocket-api-for-binance#signed-request-example-hmac    
    params.insert({"apiKey", apiKey});
    std::string payload = generatePayload(params);
    std::string signature = generateHMACSignature(secretKey, payload);
    params.insert({"signature", signature});
    return;

}
std::string RequestsHelper::getTimestamp(){
    auto now = std::chrono::system_clock::now();
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    return std::to_string(milliseconds);
}

std::string RequestsHelper::generateRequestId(){
    boost::uuids::uuid uuid = boost::uuids::random_generator()();
    std::string requestId = boost::uuids::to_string(uuid);
    return requestId;
}

std::string RequestsHelper::generateHMACSignature(const std::string& secretKey,const std::string& payload)
{
    unsigned char* digest;
    digest = HMAC(EVP_sha256(), secretKey.c_str(), secretKey.length(), (unsigned char*)payload.c_str(), payload.length(), NULL, NULL);
    char mdString[SHA256_DIGEST_LENGTH * 2 + 1];//TODO why sha length ?
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(&mdString[i * 2], "%02x", (unsigned int)digest[i]);
    }
    return std::string(mdString);
}

std::string RequestsHelper::generatePayload(const std::map<std::string, std::string>& params) {
    std::vector<std::string> param_list;
    
    for (const auto& param : params) {
        param_list.push_back(fmt::format("{}={}", param.first, param.second));
    }    
    return fmt::format("{}", fmt::join(param_list, "&"));
}