#include "bnb/utils/BNBRequests.h"

BNBRequests::BNBRequests(const std::string& apiKey, const std::string& secretKey):
    apiKey_(apiKey), secretKey_(secretKey)
{

}

request BNBRequests::logIn() {
    std::string requestId = generateRequestId();
//    auto timestamp = getTimestamp();
    std::string timestamp{"1728244231641"};

    std::map<std::string, std::string> data{
        {"apiKey", apiKey_},
        {"timestamp", timestamp}
    };
    std::string signature = generateED25519Signature(secretKey_, data);

    nlohmann::json logInRequest = {
        {"id", requestId},
        {"method", "session.logon"},
        {"params", {
            {"apiKey", apiKey_},
            {"signature", signature},
            {"timestamp", timestamp}
        }}
    };
    loggedIn_ = true; // this is inexact as logged in should wait for response
    // this flag is more indicative of wether we should sign requests or not not logged in
    return std::make_pair(requestId, logInRequest.dump());
}

request BNBRequests::querySessionStatus() {
    std::string requestId = generateRequestId();
    nlohmann::json statusRequest = {
        {"id", requestId},
        {"method", "session.status"}
    };

    return std::make_pair(requestId, statusRequest.dump());
}

request BNBRequests::logOut() {
    std::string requestId = generateRequestId();
    nlohmann::json logOutRequest = {
        {"id", requestId},
        {"method", "session.logout"}
    };

    return std::make_pair(requestId, logOutRequest.dump());
}

request BNBRequests::getAccountInformation() {
    std::string requestId = generateRequestId();
    auto timestamp = getTimestamp();
    std::map<std::string, std::string> params{
            {"timestamp", timestamp},
            {"omitZeroBalances", "true"}
    };
    if(!loggedIn_)
    {
        signRequestHMAC(params);
    }

    nlohmann::json request = {
        {"id", requestId},
        {"method", "account.status"},
        {"params", params}
    };
    return std::make_pair(requestId, request.dump());
}

request BNBRequests::placeOrder(const std::string& symbol, const std::string& side, double quantity, double price) {
    std::string requestId = generateRequestId();
    std::map<std::string, std::string> params{
            {"symbol", symbol},
            {"side", side},
            {"type", "LIMIT"},
            {"timeInForce", "GTC"},
            {"price", std::to_string(price)},
            {"quantity", std::to_string(quantity)},
            {"timestamp", getTimestamp()}
    };
    if(!loggedIn_)
    {
        signRequestHMAC(params);
    }

    nlohmann::json request = {
        {"id", requestId},
        {"method", "order.place"},
        {"params", params}
    };
    return std::make_pair(requestId, request.dump());
}

request BNBRequests::cancelAllOpenOrders(const std::string& symbol) {
    std::string requestId = generateRequestId();
    nlohmann::json request = {
        {"id", requestId},
        {"method", "openOrders.cancelAll"},
        {"params", {
            {"symbol", symbol},
            {"timestamp", getTimestamp()}
        }}
    };
    return std::make_pair(requestId, request.dump());
}

request BNBRequests::getExchangeInfo(const std::vector<std::string>& symbols) {
    std::string requestId = generateRequestId();

    nlohmann::json params = nlohmann::json::object();
    if (!symbols.empty()) {
        params["symbols"] = symbols;
    }
    nlohmann::json request = {
        {"id", requestId},
        {"method", "exchangeInfo"},
        {"params", params}  // This will be either an empty object or contain the symbols field
    };
    return std::make_pair(requestId, request.dump());
}

/*
request BNBRequests::executeOrders(const std::vector<Order>& orders)
{
    return {};
}
*/


std::string BNBRequests::getTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    return std::to_string(milliseconds);
}

std::string generateHMACSignature(const std::string& secretKey,const std::string& payload)
{
    unsigned char* digest;
    digest = HMAC(EVP_sha256(), secretKey.c_str(), secretKey.length(), (unsigned char*)payload.c_str(), payload.length(), NULL, NULL);
    char mdString[SHA256_DIGEST_LENGTH * 2 + 1];
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(&mdString[i * 2], "%02x", (unsigned int)digest[i]);
    }
    return std::string(mdString);
}

std::string generatePayload(const std::map<std::string, std::string>& params) {
    std::vector<std::string> param_list;
    
    for (const auto& param : params) {
        param_list.push_back(fmt::format("{}={}", param.first, param.second));
    }    
    return fmt::format("{}", fmt::join(param_list, "&"));
}


void BNBRequests::signRequestHMAC(std::map<std::string, std::string>& params) {
    // https://developers.binance.com/docs/binance-spot-api-docs/web-socket-api/public-websocket-api-for-binance#signed-request-example-hmac    
    params.insert({"apiKey", apiKey_});
    std::string payload = generatePayload(params);
    std::string signature = generateHMACSignature(secretKey_, payload);
    params.insert({"signature", signature});
    return;
}



std::string BNBRequests::generateRequestId() {
    boost::uuids::uuid uuid = boost::uuids::random_generator()();
    std::string requestId = boost::uuids::to_string(uuid);
    return requestId;
}


unsigned char* readPemToDer(const std::string &pemFilePath, int &derLen) {
    // Open the PEM file
    FILE *pemFile = fopen(pemFilePath.c_str(), "r");
    if (!pemFile) {
        throw std::runtime_error("Unable to open PEM file.");
    }

    // Read the private key from the PEM file
    EVP_PKEY *pkey = PEM_read_PrivateKey(pemFile, nullptr, nullptr, nullptr);
    fclose(pemFile);

    if (!pkey) {
        throw std::runtime_error("Failed to read private key from PEM file.");
    }

    // Ensure the key is Ed25519
    if (EVP_PKEY_base_id(pkey) != EVP_PKEY_ED25519) {
        EVP_PKEY_free(pkey);
        throw std::runtime_error("Not an Ed25519 key.");
    }

    // Convert the private key to DER format
    derLen = i2d_PrivateKey(pkey, nullptr);  // Get the length of the DER key
    if (derLen <= 0) {
        EVP_PKEY_free(pkey);
        throw std::runtime_error("Failed to determine DER key length.");
    }

    // Allocate memory for DER-encoded key
    unsigned char *der = new unsigned char[derLen];

    // Convert the key and store it in the buffer
    unsigned char *derPtr = der;
    if (i2d_PrivateKey(pkey, &derPtr) <= 0) {
        EVP_PKEY_free(pkey);
        delete[] der;
        throw std::runtime_error("Failed to convert private key to DER.");
    }

    // Clean up
    EVP_PKEY_free(pkey);

    return der;  // Return the DER-encoded key
}

std::string sign_payload(const std::string& privateKey, const std::string& payload) {
    unsigned char signature[crypto_sign_ed25519_BYTES];
    unsigned long long signature_len;
    int sizee;
    const unsigned char* key = readPemToDer("/home/iyedexe/workbench/rtex/config/bnb_priv_ed25519.txt", sizee); 
    LOG_WARNING("Signature : {}", std::string(reinterpret_cast<const char*>(key)));

    if (crypto_sign_detached(
        signature, &signature_len,
        reinterpret_cast<const unsigned char*>(payload.c_str()), payload.size(),
        key
        ) != 0)
    {
        throw std::runtime_error("Failed to sign message");        
    }
    LOG_WARNING("Raw signature {}", std::string(reinterpret_cast<char*>(signature)));
    
    char base64_signature[crypto_sign_ed25519_BYTES * 2];
    if (sodium_bin2base64(
        base64_signature, sizeof(base64_signature),
        signature, signature_len,
        sodium_base64_VARIANT_ORIGINAL
        ) != 0)
    {
        throw std::runtime_error("Failed to encode base64 key");        
    }

    return std::string(base64_signature);
}


std::string BNBRequests::generateED25519Signature(const std::string& secretKey, std::map<std::string, std::string>& params) {
    if (sodium_init() < 0) {
        throw std::runtime_error("Failed to initialize libsodium");
    }
    std::string payload = generatePayload(params);

    std::string signature = sign_payload(secretKey, payload);
    LOG_WARNING("Signature : {}", signature);
    return signature;
}
