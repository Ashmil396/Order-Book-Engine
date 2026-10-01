#pragma once
#include <map>
#include <unordered_map>
#include <vector>
#include <optional>
#include "Order.h"
#include "PriceLevel.h"

// A completed trade — what the engine emits when two orders match
struct Trade
{
    OrderId buyOrderId;
    OrderId sellOrderId;
    Price price;
    Quantity quantity;
};

class OrderBook
{
public:
    // The main entry point: match what you can, rest the remainder.
    // Returns every trade this order caused.
    std::vector<Trade> addOrder(Order order);

    bool cancelOrder(OrderId id);

    // Best prices — empty if that side of the book has no orders
    std::optional<Price> bestBid() const;
    std::optional<Price> bestAsk() const;

    void printBook() const;

private:
    struct OrderLocation
    {
        Side side;
        Price price;
    };

    std::map<Price, PriceLevel, std::greater<Price>> bids_;
    std::map<Price, PriceLevel> asks_;
    std::unordered_map<OrderId, OrderLocation> orderIndex_;

    uint64_t nextTimestamp_ = 1;
};