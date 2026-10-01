#include "OrderBook.h"
#include <algorithm>
#include <iostream>

std::optional<Price> OrderBook::bestBid() const
{
    if (bids_.empty())
        return std::nullopt;
    return bids_.begin()->first;
}

std::optional<Price> OrderBook::bestAsk() const
{
    if (asks_.empty())
        return std::nullopt;
    return asks_.begin()->first;
}

std::vector<Trade> OrderBook::addOrder(Order order)
{
    std::vector<Trade> trades;
    order.timestamp = nextTimestamp_++;

    if (order.side == Side::Buy)
    {
        // Match against the asks, best (lowest) price first
        while (order.quantity > 0 && !asks_.empty() && order.price >= asks_.begin()->first)
        {

            auto levelIt = asks_.begin();
            Price tradePrice = levelIt->first;
            PriceLevel &level = levelIt->second;

            Order &resting = level.frontOrder();
            Quantity traded = std::min(order.quantity, resting.quantity);

            // Read everything we need BEFORE the order can be destroyed
            OrderId restingId = resting.id;
            bool restingFullyFilled = (traded == resting.quantity);

            trades.push_back({order.id, restingId, tradePrice, traded});

            order.quantity -= traded;

            if (restingFullyFilled)
            {
                orderIndex_.erase(restingId);
                level.popFront();
            }
            else
            {
                level.reduceFront(traded);
            }

            if (level.empty())
            {
                asks_.erase(levelIt);
            }
        }

        // Anything left rests in the book
        if (order.quantity > 0)
        {
            bids_[order.price].addOrder(order);
            orderIndex_[order.id] = {Side::Buy, order.price};
        }
    }
    else
    {
        // Match against the bids, best (highest) price first
        while (order.quantity > 0 && !bids_.empty() && order.price <= bids_.begin()->first)
        {

            auto levelIt = bids_.begin();
            Price tradePrice = levelIt->first;
            PriceLevel &level = levelIt->second;

            Order &resting = level.frontOrder();
            Quantity traded = std::min(order.quantity, resting.quantity);

            OrderId restingId = resting.id;
            bool restingFullyFilled = (traded == resting.quantity);

            // Incoming is the seller, resting is the buyer — ids swap
            trades.push_back({restingId, order.id, tradePrice, traded});

            order.quantity -= traded;

            if (restingFullyFilled)
            {
                orderIndex_.erase(restingId);
                level.popFront();
            }
            else
            {
                level.reduceFront(traded);
            }

            if (level.empty())
            {
                bids_.erase(levelIt);
            }
        }

        if (order.quantity > 0)
        {
            asks_[order.price].addOrder(order);
            orderIndex_[order.id] = {Side::Sell, order.price};
        }
    }

    return trades;
}

bool OrderBook::cancelOrder(OrderId id)
{
    auto it = orderIndex_.find(id);
    if (it == orderIndex_.end())
        return false; // unknown or already gone

    const OrderLocation loc = it->second;
    orderIndex_.erase(it);

    if (loc.side == Side::Buy)
    {
        auto levelIt = bids_.find(loc.price);
        if (levelIt == bids_.end())
            return false;
        levelIt->second.removeOrder(id);
        if (levelIt->second.empty())
            bids_.erase(levelIt);
    }
    else
    {
        auto levelIt = asks_.find(loc.price);
        if (levelIt == asks_.end())
            return false;
        levelIt->second.removeOrder(id);
        if (levelIt->second.empty())
            asks_.erase(levelIt);
    }
    return true;
}

void OrderBook::printBook() const
{
    std::cout << "----- ORDER BOOK -----\n";

    // Asks: map is ascending, so iterate in reverse to show highest first
    for (auto it = asks_.rbegin(); it != asks_.rend(); ++it)
    {
        std::cout << "ASK  " << it->first
                  << "  qty " << it->second.totalQuantity()
                  << "  (" << it->second.orderCount() << " orders)\n";
    }

    std::cout << "      ---- spread ----\n";

    // Bids: map is descending, so normal iteration shows highest first
    for (const auto &[price, level] : bids_)
    {
        std::cout << "BID  " << price
                  << "  qty " << level.totalQuantity()
                  << "  (" << level.orderCount() << " orders)\n";
    }
    std::cout << "----------------------\n";
}