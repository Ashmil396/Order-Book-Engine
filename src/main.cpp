#include <iostream>
#include "PriceLevel.h"

int main()
{
    PriceLevel level;
    level.addOrder({1, Side::Buy, 502500, 100, 1});
    level.addOrder({2, Side::Buy, 502500, 50, 2});

    std::cout << "Orders: " << level.orderCount()
              << ", total qty: " << level.totalQuantity() << "\n";

    level.reduceFront(30);
    std::cout << "After partial fill, front qty: " << level.frontOrder().quantity
              << ", total: " << level.totalQuantity() << "\n";

    level.popFront();
    std::cout << "After pop, front id: " << level.frontOrder().id
              << ", total: " << level.totalQuantity() << "\n";

    return 0;
}