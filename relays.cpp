#include "relays.h"
#include "printf.h"

static Relay relays[8] = {
    Relay(0),
    Relay(1),
    Relay(2),
    Relay(3),
    Relay(4),
    Relay(5),
    Relay(6),
    Relay(7)
};

void relays_init() {
    DDRK = 0xff;
    PORTK = 0xff;
}

Relay &relay(uint8_t num) {
    return relays[num];
}

//
// Relay class implementation
//

void Relay::open() {
    PORTK |= _BV(_relay_num);
}

void Relay::close() {
    PORTK &= ~_BV(_relay_num);
}

bool Relay::is_open() {
    return PORTK & _BV(_relay_num);
}

bool Relay::is_closed() {
    return !(PORTK & _BV(_relay_num));
}

