#ifndef EVENT_HPP
#define EVENT_HPP

#include <cstdint>

struct Event {
    enum class Type { NewOrder, CancelOrder, Trade, ModifyOrder, QuoteUpdate } type;

    uint64_t timestamp;
    uint64_t orderId;
    double price;
    uint32_t quantity;
    char action;
    bool open;

    //constructors - using initializer lists is more efficient as we skip default assignment

    //dummy
    Event() : type(Type::NewOrder), timestamp(0), orderId(0), price(0), quantity(0), action(' '), open(false) { }

    //new
    Event(uint64_t ts, uint64_t oid, double p, uint32_t q, char a)
        : type(Type::NewOrder), timestamp(ts), orderId(oid), price(p), quantity(q), action(a), open(true) { }

    //cancel
    Event(uint64_t oid) 
        : type(Type::CancelOrder), orderId(oid), open(false) { }

    //trade
    Event(uint64_t ts, uint64_t oid, double p, uint32_t q, char a, bool o) 
        : type(Type::Trade), timestamp(ts), orderId(oid), price(p), quantity(q), action(a), open(o) {}

    //modify
    Event(uint64_t oid, double p, uint32_t q) 
        : type(Type::ModifyOrder), orderId(oid), price(p), quantity(q), open(true) { }
    
    //quote
    Event(uint64_t ts, double p)
        : type(Type::QuoteUpdate), timestamp(ts), price(p) { }

    
};

#endif 