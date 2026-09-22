#pragma once
#include <cstdint>

using OrderId = uint64_t;
using Price = int64_t;
using Quantity = uint32_t;

enum class Side
{
    Buy,
    Sell
};

struct Order
{
    OrderId id;
    Side side;
    Price price;
    Quantity quantity;  // remaining quantity
    uint64_t timestamp; // sequence number for time priority
};