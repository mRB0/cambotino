#ifndef MENU_MANUALCONTROL_H_
#define MENU_MANUALCONTROL_H_

#include <stdint.h>

#include "menu.h"
#include "relays.h"

class ManualControlMenuItem : public MenuItem {

public:

    ManualControlMenuItem(MenuId id) : _camera_state(0),
                                       _item_id(id),
                                       _null_choice(MenuItemChoice(0, NULL)) {
    }
    
    virtual char const *get_label() const {
        return "Camera control";
    }
    
    virtual size_t get_num_choices() const {
        return 1;
    }
    
    virtual char const *get_selection_label(uint8_t selected_index) const {
        if (_camera_state == 0) {
            return "A: Cue camera";
        } else if (_camera_state == 1) {
            return "A: Rel / B: Abrt";
        } else {
            return "??? Press B ???";
        }
    }

    virtual MenuItemChoice const &get_choice(size_t index) const {
        return _null_choice;
    }

    virtual MenuId get_id() const {
        return _item_id;
    }

    virtual bool process_keypress(KeyState const &keys, bool *redraw) {
        if (keys.key_a()) {
            if (_camera_state == 0) {
                relay(0).close();
                _camera_state++;
            } else if (_camera_state == 1) {
                relay(1).close();
                delay(50);
                relay(1).open();
                relay(0).open();
                _camera_state = 0;
            }
            *redraw = true;
        } else if (keys.key_b()) {
            relay(1).open();
            relay(0).open();
            _camera_state = 0;
            *redraw = true;
        } else {
            *redraw = false;
        }
        return *redraw;
    }

private:

    uint8_t _camera_state;
    MenuId const _item_id;

    MenuItemChoice const _null_choice;
};

#endif
