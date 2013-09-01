#include "joypad.h"

#include <stdint.h>

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>

#include "Arduino.h"

static Joypad *volatile joypad_instance = NULL;

static uint16_t const Joypad_clk_len = 0x2;
static uint16_t const Joypad_read_delay = 0x8;

static inline void enable_timer3_interrupt() {
    TIMSK3 |= _BV(OCIE3A);
}

static inline void disable_timer3_interrupt() {
    TIMSK3 &= ~_BV(OCIE3A);
}

// Reset timer3 with a new compare interrupt value
static void reset_timer3(uint16_t compare) {
    disable_timer3_interrupt();
    OCR3A = compare;
    
    // Reset the counter
    TCNT3 = 0;

    // Clear the interrupt in case it became set while disabled
    TIFR3 |= _BV(OCF3A);

    enable_timer3_interrupt();
}

// Set up timer3 for joypad interfacing
static void setup_timer3() {
    disable_timer3_interrupt();
    TCCR3A = 0x00;
    // WGM3 = 0100 (clear timer on compare match)
    TCCR3A &= ~_BV(WGM30);
    TCCR3A &= ~_BV(WGM31);
    TCCR3B |= _BV(WGM32);
    TCCR3B &= ~_BV(WGM33);

    // CS3 = 101 = clkIO/1024 (highest prescale)
    TCCR3B |= _BV(CS32);
    TCCR3B &= ~_BV(CS31);
    TCCR3B |= _BV(CS30);
    
    OCR3A = 0xffff;
}

//
// Joypad class implementation
//

Joypad::Joypad() : input_ready(false), input_value(0), input_presses(0) {
    if (joypad_instance) {
        printf("Attempt to initialize a joypad instance when one exists");
        return;
    }
    
    joypad_instance = this;

    DDRF |= _BV(DDF5) | _BV(DDF6);
    DDRF &= ~_BV(DDF7);

    // PORTF5 = CLK out (normal high, strobe low)
    // PORTF6 = LAT out (normal low, strobe high)
    // PORTF7 = D0 in (actuated button = low)

    PORTF |= _BV(PF5);
    PORTF |= _BV(PF7); // activate pull-up (maybe not required)
    PORTF &= ~_BV(PF6);

    setup_timer3();
}

Joypad::~Joypad() {
    this->stop_listening();
    joypad_instance = NULL;
}

KeyState const Joypad::get_pressed() {
    uint8_t presses = input_presses;
    input_presses = 0;
    
    return KeyState(presses);
}

KeyState const Joypad::get_held() {
    return KeyState(input_value);
}

void Joypad::start_listening() {
    reset_timer3(Joypad_read_delay);
}

void Joypad::stop_listening() {
    disable_timer3_interrupt();
}

ISR(TIMER3_COMPA_vect) {
    // We track the next operation using this 'step' variable.  We
    // need to strobe the latch line, then toggle the clock while
    // reading data from the joypad, and then wait some time and
    // repeat.
    static uint8_t step = 0;
    
    // Stores the key state while we shift it in from the joypad.  After reading the last bit, we drop
    static uint8_t keystate = 0;
    static uint8_t laststate = 0;
    
    if (step == 0) {
        // begin read with latch strobe
        joypad_instance->lat(true);
        keystate = 0;
    } else if (step == 1) {
        // latch unstrobe
        joypad_instance->lat(false);
    } else if (step >= 2 && step <= 17) {
        // read 8 bits
        if (step % 2 == 0) {
            // strobe clock
            joypad_instance->clk(true);
        } else {
            // unstrobe clock and read
            keystate |= joypad_instance->read() << ((step - 3) / 2);
            joypad_instance->clk(false);
            
            if (step == 17) {
                // last bit read; update global state
                joypad_instance->input_value = keystate;
                joypad_instance->input_ready = true;
                
                uint8_t new_keypresses = keystate & ~laststate;
                laststate = keystate;
                joypad_instance->input_presses |= new_keypresses;
            }
        }
    }
    
    step = (step + 1) % 18;
    if (step == 0) {
        // end of read; wait some time before next read
        reset_timer3(Joypad_read_delay);
    } else if (step == 1) {
        // beginning of read; increase clock until end
        reset_timer3(Joypad_clk_len);
    }
};
