#include <vector>
#include <string>
#include "fin/Order.h"

class Signal
{
public:
    std::vector<Order> orders;
    std::string description;
    double pnl;
};