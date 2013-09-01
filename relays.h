#ifndef RELAYS_H_
#define RELAYS_H_

#include <stdint.h>
#include <avr/io.h>
#include "Arduino.h"

class Relay {
    
public:

    Relay(uint8_t num) : _relay_num(num) {}

    void open();
    void close();

    bool is_open();
    bool is_closed();
    
private:

    uint8_t const _relay_num;
};

void relays_init();
Relay &relay(uint8_t num);


#endif
