#include "menu_valvecontrol.h"

#include "constants.h"

bool ValveControlMenuItem::process_keypress(KeyState const &keys, bool *redraw) {
    if (keys.key_a()) {
        if (_valve_state == 0) {
            relay(RelayIndexValve).close();
            _valve_state++;
        } else {
            relay(RelayIndexValve).open();
            _valve_state = 0;
        }
        *redraw = true;
    }
    return *redraw;
}
