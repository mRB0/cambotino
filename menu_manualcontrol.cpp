#include "menu_manualcontrol.h"

static uint8_t const RelayIndexCueShutter = 0;
static uint8_t const RelayIndexReleaseShutter = 1;

static unsigned long const ShutterReleaseTimeMillis = 100;

bool ManualControlMenuItem::process_keypress(KeyState const &keys, bool *redraw) {
    if (keys.key_a()) {
        if (_camera_state == 0) {
            relay(RelayIndexCueShutter).close();
            _camera_state++;
        } else if (_camera_state == 1) {
            relay(RelayIndexReleaseShutter).close();
            delay(ShutterReleaseTimeMillis);
            relay(RelayIndexReleaseShutter).open();
            relay(RelayIndexCueShutter).open();
            _camera_state = 0;
        }
        *redraw = true;
    } else if (keys.key_b()) {
        relay(RelayIndexReleaseShutter).open();
        relay(RelayIndexCueShutter).open();
        _camera_state = 0;
        *redraw = true;
    } else {
        *redraw = false;
    }
    return *redraw;
}
