#include "run.h"

#include <stdint.h>

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>

#include "Arduino.h"

#include "LiquidCrystal_I2C.h"

#include "joypad.h"
#include "menu.h"
#include "menu_builder.h"

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

//
// LCD menu
//

// inline void menu_process() {
//     if (input_ready) {
//         uint8_t keys = jp_presses();
//         input_ready = false;

        
//     }
// }

//
// Main
//

void run(void) {
    setup_relay();
    setup_led();

    set_led(false);

    LiquidCrystal_I2C lcd(0x3f, 16, 2);
    lcd.init();
 
    lcd.backlight();
    lcd.clear();
    
    Joypad jp;
    jp.start_listening();


    Menu &menu = build_menu(lcd);
    menu.redraw();


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
        menu.process_keys(jp);
        
        KeyState keys = jp.get_held();
        set_led(keys.key_a());
    }
}

