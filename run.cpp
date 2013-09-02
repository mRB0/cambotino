#include "run.h"

#include <stdint.h>

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>

#include "Arduino.h"

#include "LiquidCrystal_I2C.h"

#include "printf.h"

#include "joypad.h"
#include "menu.h"
#include "menu_builder.h"
#include "relays.h"
#include "constants.h"

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

void execute_synchronized_capture() {
    unsigned long valve_time = get_valve_open_time_ms();
    unsigned long shutter_time = get_valve_to_shutter_time_ms();
    bool schedule_from_valve_open = (get_valve_shutter_reference() == MenuItemChoiceIdShutterReleasesAfterValveOpen);
    
    if (schedule_from_valve_open) {
        if (shutter_time < valve_time) {
            shutter_time = 0;
        } else {
            shutter_time -= valve_time;
        }
    }
    
    Relay &valve = relay(RelayIndexValve);
    Relay &shutter_cue = relay(RelayIndexCueShutter);
    Relay &shutter_release = relay(RelayIndexReleaseShutter);
    
    shutter_cue.close();

    delay(ShutterPrepareTimeMillis);
    
    valve.close();
    delay(valve_time);
    valve.open();

    delay(shutter_time);
    shutter_release.close();

    delay(ShutterReleaseTimeMillis);
    
    shutter_release.open();
    shutter_cue.open();
}

void run(void) {
    setup_led();
    
    relays_init();
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
    menu.redraw(true);

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
        KeyState pressed = menu.process_keys(jp);

        if (pressed.key_start()) {
            lcd.clear();
            lcd.print("  Capturing...  ");
            execute_synchronized_capture();
            menu.redraw(true);
        }
    }
}

