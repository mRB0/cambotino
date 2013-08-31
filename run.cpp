#include "run.h"

#include <stdint.h>

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>

#include "Arduino.h"

/*
 * # Port definitions
 *
 * ## Button
 *
 * Pin 14: button yellow (input)
 * Pin 15: button red (input)
 * Pin 16: button white (output)
 *
 * Half-press connects yellow and white;
 * full-press connects yellow, white, and red.
 *
 * We'll set yellow and red as inputs with internal pull-ups on.
 * White will be a low output; when we read yellow/red as low, that
 * indicates that the corresponding buttons are pressed.
 *
 * Pins 14 and 15 (PORTJ[1,0]) are pin-change interrupts 10 and 9,
 * which we can use to detect button presses.
 *
 * ## Timer3
 *
 * Timer3 is being used as a button debounce timer.  It's activated
 * when a pin change interrupt is received, and when it fires we read
 * the new button state and deactivate it.
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

inline void enable_pcint1_interrupt() {
    PCICR |= _BV(PCIE1);
}

inline void disable_pcint1_interrupt() {
    PCICR &= ~_BV(PCIE1);
}

// Set up button with two states (half and full press)
void setup_button() {
    disable_pcint1_interrupt();
    
    DDRH |= _BV(DDH1); // PH1 (16) out
    PORTH &= ~_BV(PH1); // output low
    
    DDRJ &= ~_BV(DDJ1); // PJ1 (14) in; PCINT10
    PORTJ |= _BV(PJ1); // enable pull-up
    DDRJ &= ~_BV(DDJ0); // PJ0 (15) in; PCINT9
    PORTJ |= _BV(PJ0); // enable pull-up

    // enable interrupts on PJ1 (14) and PJ0 (15)
    PCMSK1 |= _BV(PCINT9);
    PCMSK1 |= _BV(PCINT10);
    
    // clear interrupt flag
    PCIFR |= _BV(PCIF1);
}

// Set up timer3 used for button debounce
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
    
    // Preload a large value, although a new value will be set every
    // time the compare interrupt is enabled anyway.
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

inline void reset_button() {
    disable_pcint1_interrupt();
    PCIFR |= _BV(PCIF1);
    enable_pcint1_interrupt();
}

//
// Main
//

volatile uint8_t button_state = 0;
inline void read_button() {
    button_state = PINJ & (_BV(PJ0) | _BV(PJ1));
}

inline bool btn1() {
    return !(button_state & _BV(PJ1));
}

inline bool btn2() {
    return !(button_state & _BV(PJ0));
}

void run(void) {
    setup_relay();
    setup_led();
    setup_button();
    setup_timer3();

    // Disable timer0 overflow interrupt (Arduino uses timer0 for its
    // delay functions)
    TIMSK0 &= ~_BV(TOIE0);

    // We only want IDLE sleep mode, so that we can keep Timer3
    // running.
    set_sleep_mode(SLEEP_MODE_IDLE);
    
    // Read initial button state
    reset_button();
    read_button();
    
    cli();
    for(;;) {
        // Activate relays on button press:
        // Half press = relay 1 only
        // Full press = relay 1 & 2
        activate_relay(0, btn1() || btn2());
        activate_relay(1, btn1() && btn2());

        // LED on when pressed halfway, off when pressed fully or not
        // pressed at all
        set_led(btn1() && !btn2());

        // Sleep until next interrupt
        sleep_enable();
        sei();
        sleep_cpu();
        cli();
        sleep_disable();
    }
}

//
// ISRs
//

ISR(TIMER3_COMPA_vect) {
    // Read button state
    read_button();

    // Cancel timer
    disable_timer3_interrupt();
};

ISR(PCINT1_vect) {
    // Start (or re-start) the debounce timer
    reset_timer3(0x40);

    // We could disable the pin-change interrupt here, and re-enable
    // when the timer fires.
    //
    // If we did that, the timer would fire exactly 0x40 ticks after
    // the first "bounce" and we would read the button state at that
    // point.
    //
    // Because we don't, the timer will fire 0x40 ticks after the LAST
    // bounce, and we'll read the button state then instead.
    //
    // Practically speaking, it's unlikely to make much difference.
};
