#ifndef RELAYS_H_
#define RELAYS_H_

#include <stdint.h>
#include <avr/io.h>
#include "Arduino.h"

class Relay {
    
public:

    Relay(uint8_t volatile *port, uint8_t volatile *ddr, uint8_t pin, bool invert)
        : _port(port), _pin(pin), _invert(invert) {
        *ddr |= _BV(pin);
        open();
    }

    void open();
    void close();

    bool is_open();
    bool is_closed();
    
private:

    void drop();
    void raise();
    
    uint8_t volatile *const _port;
    uint8_t const _pin;
    bool const _invert;
};

Relay &relay(uint8_t num);


#endif
