#include "run.h"

#include <stdint.h>

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>

#include "Arduino.h"

#include "LiquidCrystal_I2C.h"

extern "C" {
#include "printf.h"
}

#include "joypad.h"
#include "menu.h"
#include "menu_builder.h"
#include "relays.h"

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

// Set up Arduino onboard LED at PORTB7
void setup_led() {
    DDRB |= _BV(DDB7);
    PORTB &= ~_BV(PB7);
}

void set_led(bool on) {
    if (on) {
        PORTB |= _BV(PB7);
    } else {
        PORTB &= ~_BV(PB7);
    }
}

//
// Main
//

extern "C" {
    void serial_putc(void *p, char c) {
        Serial.print(c);
        Serial.flush();
    }
}

void run(void) {
    relays_init();
    
    setup_led();
    Serial.begin(9600);
    init_printf(NULL, serial_putc);
    
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

    uint8_t camera_state = 0;
    
    for(;;) {
        KeyState pressed = menu.process_keys(jp);

        // Manual camera operation
        if (pressed.key_start()) {
            if (camera_state == 0) {
                relay(0).close();
                camera_state++;
            } else if (camera_state == 1) {
                relay(1).close();
                delay(50);
                relay(1).open();
                relay(0).open();
                camera_state = 0;
            }
        } else if (pressed.key_select()) {
            relay(1).open();
            relay(0).open();
            camera_state = 0;
        }
    }
}

