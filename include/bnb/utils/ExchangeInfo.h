#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include "bnb/utils/SymbolFilter.h"
#include "fin/Symbol.h"
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <iostream>
using json = nlohmann::json;

struct SymbolJson {
    std::string name;
    std::string status;
    std::string baseAsset;
    std::string quoteAsset;
    std::vector<std::map<std::string, std::string>> filters;
};

class ExchangeInfo {
private:
    std::vector<SymbolJson> symbols_;

public:
    explicit ExchangeInfo(const nlohmann::json& jsonData);

    std::vector<Symbol> getSymbols() const;
    std::vector<Symbol> getRelatedSymbols(std::string asset) const;

    SymbolFilter createSymbolFilter(const std::string& symbolName) const;
};