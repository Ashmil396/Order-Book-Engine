#include <iostream>
#include "Order.h"

void printOrder(const Order &o)
{
    std::cout << "Order " << o.id
              << (o.side == Side::Buy ? " BUY" : " SELL")
              << " price " << o.price
              << " qty " << o.quantity << "\n";
}

int main()
{
    Order buy{1, Side::Buy, 502500, 100, 1};

    printOrder(buy);

    return 0;
}