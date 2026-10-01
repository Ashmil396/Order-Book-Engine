# Order Book Engine

A limit order book and matching engine written in C++17, modelled on how real equity exchanges match orders. Incoming orders are matched using **price-time priority**: best price first, and the oldest order first within a price.

> **Status:** Work in progress. Core matching, cancellation, and book display are implemented. Testing, an O(1) cancel path, and a market simulator are in progress (see [Roadmap](#roadmap)).

## Features

- Limit orders on both sides of the book (buy and sell)
- Price-time priority matching
- Partial fills, including a single order sweeping multiple price levels
- Trades execute at the **resting** order's price, so an aggressive order can get price improvement
- Cancel by order ID
- Best bid / best ask queries
- Book display with asks above the spread and bids below it

## Build and run

Requires `g++` with C++17 support (tested on Ubuntu via WSL).

```bash
g++ -std=c++17 -Wall -Wextra -Iinclude src/*.cpp -o engine
./engine
```

## Project structure

```
order-book-engine/
├── include/
│   ├── Order.h        # Order struct, type aliases, Side enum
│   ├── PriceLevel.h   # FIFO queue of orders at a single price
│   └── OrderBook.h    # Both sides of the book, Trade struct
└── src/
    ├── PriceLevel.cpp
    ├── OrderBook.cpp  # Matching logic
    └── main.cpp       # Demo driver
```

## Design decisions

### Prices are integers, not floating point
Prices are stored as integer ticks (for example, 50.25 is stored as `502500`). Floating-point types can't represent most decimals exactly, so two prices that should be equal might not compare equal. That would silently break a book keyed by price. This mirrors the fixed-point `Price(4)` field in Nasdaq's TotalView-ITCH 5.0 protocol.

### Each side of the book is a `std::map` with a different sort order
```cpp
std::map<Price, PriceLevel, std::greater<Price>> bids_;  // descending
std::map<Price, PriceLevel>                      asks_;  // ascending
```
A balanced tree keeps price levels sorted, with O(log n) insert and erase. Reversing the comparator on the bid side means `begin()` returns the best price on **both** sides in O(1), so the matching code never has to special-case which side it is reading.

### Orders at one price are kept in a `std::list`
Within a price level, orders fill oldest first, so the level is a FIFO queue. I compared three containers:

| Operation           | `vector` | `deque` | `list` + hash map |
|---------------------|----------|---------|-------------------|
| Append              | O(1)*    | O(1)    | O(1)              |
| Fill (pop front)    | O(n)     | O(1)    | O(1)              |
| Cancel by ID        | O(n)     | O(n)    | O(1)              |
| Iterators stay valid| No       | No      | Yes               |

On real exchanges, cancels make up most of the message traffic, so cancel by ID needs to be fast. Only `std::list` keeps iterators valid when other elements are inserted or removed, which is what allows a hash map of iterators to jump straight to any order. The trade-off is weaker cache locality. That matters little here, because the engine only touches the front of a level or a directly indexed order.

### Cached level quantity
Each `PriceLevel` keeps a running `totalQuantity_` rather than summing its orders on every query. This makes depth lookups O(1). The trade-off is an invariant that every mutating method must maintain, enforced with assertions.

## Current limitations

- **Cancel is O(n) within a price level.** `orderIndex_` currently stores only the side and price of each order, so a cancel finds the right level in O(1) but then scans it. The fix is to store a `std::list<Order>::iterator` in the index, making cancellation fully O(1).
- Single instrument only.
- Limit orders only (no market, IOC, or FOK orders yet).
- Bid and ask matching logic is duplicated, because the two maps have different types. A templated helper would remove the duplication.

## Roadmap

- [x] Order data model
- [x] Price level FIFO queue
- [x] Matching engine with partial fills
- [x] Cancel and book display
- [ ] Edge-case test suite
- [ ] O(1) cancel via stored list iterators
- [ ] Interactive CLI (`add`, `cancel`, `book`)
- [ ] Agent-based market simulator (noise traders, market maker, momentum trader)
- [ ] Live visualization of book depth and price
- [ ] Strategy backtesting with PnL tracking
- [ ] Throughput and latency benchmarks

## References

- Nasdaq TotalView-ITCH 5.0 specification
- Larry Harris, *Trading and Exchanges: Market Microstructure for Practitioners*