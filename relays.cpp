#include "relays.h"
#include "printf.h"

static Relay relays[] = {
    Relay(&PORTH, &DDRH, PORTH0, true),
    Relay(&PORTD, &DDRD, PORTD3, true),
    Relay(&PORTD, &DDRD, PORTD2, false)
};

Relay &relay(uint8_t num) {
    return relays[num];
}

//
// Relay class implementation
//

void Relay::drop() {
    *_port &= ~_BV(_pin);
}

void Relay::raise() {
    *_port |= _BV(_pin);
}

void Relay::open() {
    _invert ? raise() : drop();
}

void Relay::close() {
    _invert ? drop() : raise();
}

bool Relay::is_open() {
    return (bool)(!!(*_port & _BV(_pin))) == _invert;
}

bool Relay::is_closed() {
    return (bool)(!(*_port & _BV(_pin))) == _invert;
}

