#include <string>
#include <vector>
#include <map> 

#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <fmt/ranges.h>

class RequestsHelper
{
public:
    static std::string generateED25519Signature(const std::string& secretKey, std::map<std::string, std::string>& params);
    static void signRequestHMAC(std::map<std::string, std::string>& params, const std::string& apiKey ,const std::string& secretKey);
    static std::string getTimestamp();
    static std::string generateRequestId();

protected:
    static std::string generateHMACSignature(const std::string& secretKey,const std::string& payload);
    static std::string generatePayload(const std::map<std::string, std::string>& params);
    
};