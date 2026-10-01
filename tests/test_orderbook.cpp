#include <iostream>
#include <vector>
#include "OrderBook.h"

static int passed = 0, failed = 0;

#define CHECK(cond)                                                     \
    do                                                                  \
    {                                                                   \
        if (cond)                                                       \
        {                                                               \
            ++passed;                                                   \
        }                                                               \
        else                                                            \
        {                                                               \
            ++failed;                                                   \
            std::cout << "  FAIL line " << __LINE__ << ": " #cond "\n"; \
        }                                                               \
    } while (0)

// Helper: check one trade's fields in a single line
static bool tradeIs(const Trade &t, OrderId buy, OrderId sell, Price p, Quantity q)
{
    return t.buyOrderId == buy && t.sellOrderId == sell && t.price == p && t.quantity == q;
}

// 1. A buy that sweeps three ask levels and rests the remainder
void testSweepThreeLevels()
{
    std::cout << "testSweepThreeLevels\n";
    OrderBook book;
    book.addOrder({1, Side::Sell, 100, 10, 0});
    book.addOrder({2, Side::Sell, 101, 10, 0});
    book.addOrder({3, Side::Sell, 102, 10, 0});

    auto trades = book.addOrder({4, Side::Buy, 102, 35, 0});

    CHECK(trades.size() == 3);
    CHECK(tradeIs(trades[0], 4, 1, 100, 10)); // best price first
    CHECK(tradeIs(trades[1], 4, 2, 101, 10));
    CHECK(tradeIs(trades[2], 4, 3, 102, 10));
    CHECK(!book.bestAsk().has_value()); // ask side fully cleared
    CHECK(book.bestBid() == 102);       // leftover 5 rests as a bid
}

// 2. An order that exactly empties a level: the level must disappear
void testExactlyEmptiesLevel()
{
    std::cout << "testExactlyEmptiesLevel\n";
    OrderBook book;
    book.addOrder({1, Side::Sell, 100, 50, 0});
    book.addOrder({2, Side::Sell, 105, 50, 0});

    auto trades = book.addOrder({3, Side::Buy, 100, 50, 0});

    CHECK(trades.size() == 1);
    CHECK(tradeIs(trades[0], 3, 1, 100, 50));
    CHECK(book.bestAsk() == 105);       // 100 level erased
    CHECK(!book.bestBid().has_value()); // nothing left to rest
}

// 3. Time priority: same price, older order fills first
void testTimePriority()
{
    std::cout << "testTimePriority\n";
    OrderBook book;
    book.addOrder({1, Side::Buy, 100, 10, 0});
    book.addOrder({2, Side::Buy, 100, 10, 0});

    auto trades = book.addOrder({3, Side::Sell, 100, 15, 0});

    CHECK(trades.size() == 2);
    CHECK(tradeIs(trades[0], 1, 3, 100, 10)); // order 1 arrived first
    CHECK(tradeIs(trades[1], 2, 3, 100, 5));  // order 2 partially filled
}

// 4. Price improvement: trade happens at the RESTING price
void testPriceImprovement()
{
    std::cout << "testPriceImprovement\n";
    OrderBook book;
    book.addOrder({1, Side::Sell, 100, 10, 0});

    auto trades = book.addOrder({2, Side::Buy, 110, 10, 0});

    CHECK(trades.size() == 1);
    CHECK(trades[0].price == 100); // not 110
}

// 5. No cross: both orders rest, spread stays open
void testNoCross()
{
    std::cout << "testNoCross\n";
    OrderBook book;
    auto t1 = book.addOrder({1, Side::Buy, 99, 10, 0});
    auto t2 = book.addOrder({2, Side::Sell, 101, 10, 0});

    CHECK(t1.empty());
    CHECK(t2.empty());
    CHECK(book.bestBid() == 99);
    CHECK(book.bestAsk() == 101);
}

// 6. Cancel from the MIDDLE of a queue: later orders keep their place
void testCancelMiddleOfQueue()
{
    std::cout << "testCancelMiddleOfQueue\n";
    OrderBook book;
    book.addOrder({1, Side::Sell, 100, 10, 0});
    book.addOrder({2, Side::Sell, 100, 10, 0});
    book.addOrder({3, Side::Sell, 100, 10, 0});

    CHECK(book.cancelOrder(2));

    auto trades = book.addOrder({4, Side::Buy, 100, 20, 0});
    CHECK(trades.size() == 2);
    CHECK(tradeIs(trades[0], 4, 1, 100, 10));
    CHECK(tradeIs(trades[1], 4, 3, 100, 10)); // 2 skipped: it was cancelled
    CHECK(!book.bestAsk().has_value());
}

// 7. Cancel an unknown ID, and cancel the same ID twice
void testCancelUnknownAndTwice()
{
    std::cout << "testCancelUnknownAndTwice\n";
    OrderBook book;
    book.addOrder({1, Side::Buy, 100, 10, 0});

    CHECK(!book.cancelOrder(999)); // never existed
    CHECK(book.cancelOrder(1));
    CHECK(!book.cancelOrder(1));        // already gone
    CHECK(!book.bestBid().has_value()); // level erased with its last order
}

// 8. Cancel an order that was already fully filled
void testCancelFilledOrder()
{
    std::cout << "testCancelFilledOrder\n";
    OrderBook book;
    book.addOrder({1, Side::Sell, 100, 10, 0});
    book.addOrder({2, Side::Buy, 100, 10, 0}); // fills order 1 completely

    CHECK(!book.cancelOrder(1)); // filled orders can't be cancelled
}

int main()
{
    testSweepThreeLevels();
    testExactlyEmptiesLevel();
    testTimePriority();
    testPriceImprovement();
    testNoCross();
    testCancelMiddleOfQueue();
    testCancelUnknownAndTwice();
    testCancelFilledOrder();

    std::cout << "\n"
              << passed << " passed, " << failed << " failed\n";
    return failed == 0 ? 0 : 1;
}