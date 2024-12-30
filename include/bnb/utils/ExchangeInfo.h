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
#include <unordered_map>
using json = nlohmann::json;

class ExchangeInfo {
private:
    std::vector<Symbol> symbols_;

    SymbolFilter createSymbolFilter(const json& filterJson) const;

public:
    explicit ExchangeInfo(const nlohmann::json& jsonData);

    std::vector<Symbol> getSymbols() const;
    std::vector<Symbol> getRelatedSymbols(std::string asset) const;

};