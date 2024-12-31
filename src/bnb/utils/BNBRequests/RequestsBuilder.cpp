#include "bnb/utils/BNBRequests/RequestsBuilder.h"

request RequestsBuilder::basicRequest(const std::string& method)
{
    std::string requestId = RequestsHelper::generateRequestId();
    nlohmann::json requestBody = {
        {"id", requestId},
        {"method", method}
    };

    return std::make_pair(requestId, requestBody.dump());
}

request RequestsBuilder::paramsUnsignedRequest(const std::string& method, const nlohmann::json& params)
{
    std::string requestId = RequestsHelper::generateRequestId();
    nlohmann::json requestBody = {
        {"id", requestId},
        {"method", method},
        {"params", params}
    };

    return std::make_pair(requestId, requestBody.dump());
}

request RequestsBuilder::paramsSignedRequest(const std::string& method, std::map<std::string, std::string>& params)
{
    if (instance == nullptr) {
        std::cout << "Singleton is not yet initialized.\n";
        return std::make_pair("", "");
    }
    params["timestamp"] = RequestsHelper::getTimestamp();
    RequestsHelper::signRequestHMAC(params, instance->apiKey_, instance->secretKey_);

    std::string requestId = RequestsHelper::generateRequestId();
    nlohmann::json requestBody = {
        {"id", requestId},
        {"method", method},
        {"params", params}
    };

    return std::make_pair(requestId, requestBody.dump());
}

/*

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
*/