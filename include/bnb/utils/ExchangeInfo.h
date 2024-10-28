// ExchangeInfo.h
#ifndef EXCHANGE_INFO_H
#define EXCHANGE_INFO_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include "bnb/utils/SymbolFilter.h"
#include "fin/Symbol.h"

struct SymbolJson {
    std::string name;
    std::string status;
    std::string baseAsset;
    std::string quoteAsset;
    std::vector<std::map<std::string, std::string>> filters;
};

class ExchangeInfo {
private:
    std::vector<SymbolJson> symbols;

public:
    ExchangeInfo(const std::string& jsonData);

    std::vector<Symbol> getSymbols() const;
    std::vector<Symbol> getRelatedSymbols(std::string asset) const;

    SymbolFilter createSymbolFilter(const std::string& symbolName) const;
};

#endif // EXCHANGE_INFO_H
