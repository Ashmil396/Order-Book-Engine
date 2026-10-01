#include "PriceLevel.h"
#include <cassert>

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
    assert(!orders_.empty() && "popFront called on empty PriceLevel");
    totalQuantity_ -= orders_.front().quantity;
    orders_.pop_front();
}

void PriceLevel::reduceFront(Quantity qty)
{
    assert(qty <= orders_.front().quantity && "reduceFront would underflow"); // used assertion to ensure we don't reduce more than available
    orders_.front().quantity -= qty;
    totalQuantity_ -= qty;
}

bool PriceLevel::removeOrder(OrderId id)
{
    for (auto it = orders_.begin(); it != orders_.end(); ++it)
    {
        if (it->id == id)
        {
            totalQuantity_ -= it->quantity;
            orders_.erase(it);
            return true;
        }
    }
    return false;
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