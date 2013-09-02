#include "menu_valvecontrol.h"

#include "constants.h"

bool ValveControlMenuItem::process_keys(KeyState const &keys, KeyState const &held_keys) {
    if (keys.key_select()) {
        if (_valve_state == 0) {
            relay(RelayIndexValve).close();
            _valve_state++;
        } else {
            relay(RelayIndexValve).open();
            _valve_state = 0;
        }
        return true;
    }
    return false;
}
