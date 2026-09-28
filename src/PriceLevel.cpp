#include "PriceLevel.h"

void PriceLevel::addOrder(const Order &order)
{
    orders_.push_back(order);
    totalQuantity_ += order.quantity;
}

Order &PriceLevel::frontOrder()
{
    return orders_.front();
}

void PriceLevel::popFront()
{
    totalQuantity_ -= orders_.front().quantity;
    orders_.pop_front();
}

void PriceLevel::reduceFront(Quantity qty)
{
    orders_.front().quantity -= qty;
    totalQuantity_ -= qty;
}

bool PriceLevel::empty() const
{
    return orders_.empty();
}

Quantity PriceLevel::totalQuantity() const
{
    return totalQuantity_;
}

size_t PriceLevel::orderCount() const
{
    return orders_.size();
}