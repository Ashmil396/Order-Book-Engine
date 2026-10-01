#pragma once
#include <list>
#include "Order.h"

class PriceLevel
{
public:
    // Add an order to the back of the queue (newest = lowest priority)
    void addOrder(const Order &order);

    // The oldest order at this level — the next one to get filled
    Order &frontOrder();

    bool removeOrder(OrderId id);

    // Remove the oldest order (it got fully filled)
    void popFront();

    // Reduce the oldest order's quantity by qty (partial fill)
    void reduceFront(Quantity qty);

    bool empty() const;
    Quantity totalQuantity() const; // sum of all orders at this level
    size_t orderCount() const;

private:
    std::list<Order> orders_;
    Quantity totalQuantity_ = 0; // kept in sync, so we never sum the list
};