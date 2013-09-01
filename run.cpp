#include "run.h"

#include <stdint.h>

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>

#include "Arduino.h"

#include "LiquidCrystal_I2C.h"

static uint16_t const Joypad_clk_len = 0x20;
static uint16_t const Joypad_read_delay = 0x80;

/*
 * # Port definitions
 *
 * ## NES joypad
 *
 * Hooked up to PORTF[5:7], which maps to Arduino analog pins A5-A7.
 * This sits neatly next to the relay pins on PORTK (A8-A15).
 *
 * The NES joypad shifts 8 bits, one for each key.  Details can be
 * found at http://www.mit.edu/~tarvizo/nes-controller.html and many
 * other sources.
 *
 * PF5 = CLK out
 * PF6 = LAT out
 * PINF7 = D0 in
 *
 * ## Timer3
 *
 * Timer3 is being used to drive the NES joypad clock and latch
 * operation.
 *
 * ## Relays
 *
 * The relay board has 8 relays, each with one active-low input.  I'm
 * using all the pins of PORTK set as outputs; they occupy analog pins
 * A8-A15 on the Arduino.  I'm not using analog inputs for now and
 * it's a clean wiring job, so that's fine.  They map directly: A8 =
 * PK0, A9 = PK1, etc.
 */

inline void enable_timer3_interrupt() {
    TIMSK3 |= _BV(OCIE3A);
}

inline void disable_timer3_interrupt() {
    TIMSK3 &= ~_BV(OCIE3A);
}

// Set up timer3 for joypad interfacing
void setup_timer3() {
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

// Set up relay board
void setup_relay() {
    DDRK = 0xff;
    PORTK = 0xff;
}

// Set up Arduino onboard LED at PORTB7
void setup_led() {
    DDRB |= _BV(DDB7);
    PORTB &= ~_BV(PB7);
}

void activate_relay(uint8_t idx, bool state) {
    if (state) {
        PORTK &= ~_BV(idx);
    } else {
        PORTK |= _BV(idx);
    }
}

void set_led(bool on) {
    if (on) {
        PORTB |= _BV(PB7);
    } else {
        PORTB &= ~_BV(PB7);
    }
}

// Reset timer3 with a new compare interrupt value
void reset_timer3(uint16_t compare) {
    disable_timer3_interrupt();
    OCR3A = compare;
    
    // Reset the counter
    TCNT3 = 0;

    // Clear the interrupt in case it became set while disabled
    TIFR3 |= _BV(OCF3A);

    enable_timer3_interrupt();
}

//
// Joypad
//

// Set joypad latch value.
inline void jp_lat(boolean activate) {
    if (activate) {
        PORTF |= _BV(PF6);
    } else {
        PORTF &= ~_BV(PF6);
    }
}

// Set joypad clock value.
inline void jp_clk(boolean activate) {
    if (activate) {
        PORTF &= ~_BV(PF5);
    } else {
        PORTF |= _BV(PF5);
    }
}

// Read data bit from joypad.
inline uint8_t jp_read() {
    return !(PINF & _BV(PINF7));
}

void setup_joypad() {
    DDRF |= _BV(DDF5) | _BV(DDF6);
    DDRF &= ~_BV(DDF7);

    // PORTF5 = CLK out (normal high, strobe low)
    // PORTF6 = LAT out (normal low, strobe high)
    // PORTF7 = D0 in (actuated button = low)

    PORTF |= _BV(PF5);
    PORTF |= _BV(PF7); // activate pull-up (maybe not required)
    PORTF &= ~_BV(PF6);
}

static volatile boolean input_set = false;
static volatile uint8_t input_value = 0;

inline bool jp_key_a() { return input_value & (1 << 0); }
inline bool jp_key_b() { return input_value & (1 << 1); }
inline bool jp_key_select() { return input_value & (1 << 2); }
inline bool jp_key_start() { return input_value & (1 << 3); }
inline bool jp_key_up() { return input_value & (1 << 4); }
inline bool jp_key_down() { return input_value & (1 << 5); }
inline bool jp_key_left() { return input_value & (1 << 6); }
inline bool jp_key_right() { return input_value & (1 << 7); }

//
// Main
//

void run(void) {
    setup_relay();
    setup_led();
    setup_joypad();
    setup_timer3();

    LiquidCrystal_I2C lcd(0x3f,16,2);

    lcd.init();
 
    lcd.backlight();
    lcd.clear();

    set_led(false);
    
    reset_timer3(Joypad_read_delay);

    /*
     * Ideally, we would clear interrupts (or at least some
     * interrupts) while processing parts of the interrupt results,
     * and then put the CPU to sleep with interrupts back on.  That
     * way we can definitively avoid potential sychronization issues,
     * and respond to interrupts with a consistent response time
     * (which is not necessarily important right now).
     *
     * We can't disable interrupts altogether, because Arduino delay()
     * calls use a timer interrupt, and the LCD module depends on it.
     *
     * We should actually disable the specific interrupts selectively.
     * I'll do that eventually, but for now the code is changing so
     * much that it's not really worthwhile.
     */

    for(;;) {
        if (input_set) {
            uint8_t keys = input_value;
            input_set = false;
            
            lcd.home();

            char states[9] = "><v^+-BA";
            for(int i = 0; i < 8; i++) {
                if (!(keys & (1 << i))) {
                    states[7 - i] = ' ';
                }
            }
            lcd.print(states);
        }
    }
}

//
// ISRs
//

ISR(TIMER3_COMPA_vect) {
    // We track the next operation using this 'step' variable.  We
    // need to strobe the latch line, then toggle the clock while
    // reading data from the joypad, and then wait some time and
    // repeat.
    static uint8_t step = 0;
    
    // Stores the key state while we shift it in from the joypad.  After reading the last bit, we drop
    static uint8_t keystate = 0;

    if (step == 0) {
        // begin read with latch strobe
        jp_lat(true);
        keystate = 0;
    } else if (step == 1) {
        // latch unstrobe
        jp_lat(false);
    } else if (step >= 2 && step <= 17) {
        // read 8 bits
        if (step % 2 == 0) {
            // strobe clock
            jp_clk(true);
        } else {
            // unstrobe clock and read
            keystate |= jp_read() << ((step - 3) / 2);
            jp_clk(false);
            
            if (step == 17) {
                // last bit read; update global state
                input_value = keystate;
                input_set = true;
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
