#ifndef MENU_MANUALCONTROL_H_
#define MENU_MANUALCONTROL_H_

#include <stdint.h>

#include "menu.h"
#include "relays.h"

class ManualControlMenuItem : public MenuItem {

public:

    ManualControlMenuItem(MenuId id) : _camera_state(0),
                                       _item_id(id) {
    }
    
    virtual char const *get_label(char *label_buf, size_t buflen) const {
        return "Camera control";
    }

    // We supply one "null" choice because we don't actually have any
    // choices, but Menu requires at least one.
    virtual size_t get_num_choices() const {
        return 1;
    }
    
    virtual char const *get_selection_label(char *label_buf, size_t buflen) const {
        if (_camera_state == 0) {
            return "A: Cue shutter";
        } else if (_camera_state == 1) {
            return "A: Rel / B: Abrt";
        } else if (_camera_state == 2) {
            return "Triggered";
        } else {
            return "??? Press B ???";
        }
    }

    virtual MenuId get_id() const {
        return _item_id;
    }

    virtual bool process_keys(KeyState const &pressed_keys, KeyState const &held_keys);

private:

    uint8_t _camera_state;
    MenuId const _item_id;
};

#endif
