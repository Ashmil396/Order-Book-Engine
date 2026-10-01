#include <iostream>
#include "OrderBook.h"

int main()
{
    OrderBook book;

    // Two resting sells
    book.addOrder({1, Side::Sell, 502500, 100, 0});
    book.addOrder({2, Side::Sell, 502600, 50, 0});

    // A buy that crosses both levels
    auto trades = book.addOrder({3, Side::Buy, 502600, 120, 0});

    for (const auto &t : trades)
    {
        std::cout << "TRADE buy=" << t.buyOrderId
                  << " sell=" << t.sellOrderId
                  << " price=" << t.price
                  << " qty=" << t.quantity << "\n";
    }

    auto ask = book.bestAsk();
    std::cout << "Best ask now: " << (ask ? std::to_string(*ask) : "none") << "\n";
    return 0;
}