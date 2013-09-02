#ifndef MENU_VALVECONTROL_H_
#define MENU_VALVECONTROL_H_

#include <stdint.h>

#include "menu.h"
#include "relays.h"

class ValveControlMenuItem : public MenuItem {

public:

    ValveControlMenuItem(MenuId id) : _valve_state(0),
                                      _item_id(id) {}
    
    virtual char const *get_label() const {
        return "Valve control";
    }

    // We supply one "null" choice because we don't actually have any
    // choices, but Menu requires at least one.
    virtual size_t get_num_choices() const {
        return 1;
    }
    
    virtual char const *get_selection_label() const {
        if (_valve_state == 0) {
            return "A: Open";
        } else {
            return "A: Close";
        }
    }

    virtual SelectionValue get_selection_value() const {
        return 0;
    }
    
    virtual MenuId get_id() const {
        return _item_id;
    }

    virtual bool process_keypress(KeyState const &keys, bool *redraw);

private:

    uint8_t _valve_state;
    MenuId const _item_id;
};

#endif
