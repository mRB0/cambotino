#ifndef JOYPAD_H_
#define JOYPAD_H_

#include <stdint.h>

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>

class KeyState {

public:

    KeyState(uint8_t const keyvalue) : _keyvalue(keyvalue) {}

    uint8_t const _keyvalue;

    inline bool any() const { return _keyvalue != 0; }
    
    inline bool key_a() const { return _keyvalue & (1 << 0); }
    inline bool key_b() const { return _keyvalue & (1 << 1); }
    inline bool key_select() const { return _keyvalue & (1 << 2); }
    inline bool key_start() const { return _keyvalue & (1 << 3); }
    inline bool key_up() const { return _keyvalue & (1 << 4); }
    inline bool key_down() const { return _keyvalue & (1 << 5); }
    inline bool key_left() const { return _keyvalue & (1 << 6); }
    inline bool key_right() const { return _keyvalue & (1 << 7); }
};



class Joypad {

public:

    Joypad();
    virtual ~Joypad();

    void start_listening();
    void stop_listening();

    // Return newly pressed keys and clear the list of "newly pressed
    // keys"; ie. the next call will return no keys pressed unless one
    // was released (if necessary) and pressed again.
    KeyState const get_pressed();

    // Return keys currently being held down.
    KeyState const get_held();

    // Set joypad latch value.
    inline void lat(bool activate) {
        if (activate) {
            PORTF |= _BV(PF6);
        } else {
            PORTF &= ~_BV(PF6);
        }
    };

    // Set joypad clock value.
    inline void clk(bool activate) {
        if (activate) {
            PORTF &= ~_BV(PF5);
        } else {
            PORTF |= _BV(PF5);
        }
    }

    // Read data bit from joypad.
    inline uint8_t read() {
        return !(PINF & _BV(PINF7));
    }

    volatile bool input_ready;
    volatile uint8_t input_value;
    volatile uint8_t input_presses;

};

#endif
