#include "menu_manualcontrol.h"

#include "constants.h"

bool ManualControlMenuItem::process_keys(KeyState const &keys, KeyState const &held_keys) {
    if (keys.key_a()) {
        if (_camera_state == 0) {
            relay(RelayIndexCueShutter).close();
            _camera_state++;
        } else if (_camera_state == 1) {
            relay(RelayIndexReleaseShutter).close();
            _camera_state = 2;
        }
        return true;
    } else if (keys.key_b()) {
        relay(RelayIndexReleaseShutter).open();
        relay(RelayIndexCueShutter).open();
        _camera_state = 0;
        return true;
    } else if (_camera_state == 2) {
        if (!held_keys.key_a()) {
            relay(RelayIndexReleaseShutter).open();
            relay(RelayIndexCueShutter).open();
            _camera_state = 0;
            return true;
        }
    }

    return false;
}
