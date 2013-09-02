#include "menu_valvecontrol.h"

#include "constants.h"

bool ValveControlMenuItem::process_keys(KeyState const &keys, KeyState const &held_keys) {
    if (keys.key_select()) {
        if (_valve_state == 0) {
            relay(RelayIndexValve).close();
            _valve_state = 1;
        } else {
            relay(RelayIndexValve).open();
            _valve_state = 0;
        }
        return true;
    } else if (held_keys.key_b()) {
        if (_valve_state == 0) {
            relay(RelayIndexValve).close();
            _valve_state = 2;
            return true;
        }
    } else if (_valve_state == 2) {
        // valve was held open by 'B' key; time to close it
        relay(RelayIndexValve).open();
        _valve_state = 0;
        return true;
    }
    return false;
}
