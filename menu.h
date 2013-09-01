#ifndef MENU_H_
#define MENU_H_

#include <stdint.h>

#include "LiquidCrystal_I2C.h"

#include "joypad.h"
extern "C" {
#include "printf.h"
}

typedef uint16_t MenuId;

class MenuItemChoice {

public:
    
    MenuItemChoice(MenuId id, char const *label)
        : _id(id), _label(label) {}

    char const *get_label() const { return _label; }
    MenuId get_id() const { return _id; }
    
    MenuId const _id;
    char const *const _label;
};


class MenuItem {

public:

    virtual char const *get_label() const = 0;
    
    virtual size_t get_num_choices() const = 0;
    virtual MenuItemChoice const &get_choice(size_t index) const = 0;

    virtual size_t get_default_choice_index() const {
        return 0;
    }

    virtual MenuId get_id() const {
        return 0;
    }

    virtual char const* process_keypress(KeyState const &keys, bool *processed) const {
        *processed = false;
        return NULL;
    }
};

class ArrayMenuItem : public MenuItem {

public:

    ArrayMenuItem(MenuId id, char const *label, MenuItemChoice const *const *choices, size_t num_choices)
        : _label(label), _choices(choices), _num_choices(num_choices), _item_id(id) {
    }
    
    virtual char const *get_label() const {
        return _label;
    }
    
    virtual size_t get_num_choices() const {
        return _num_choices;
    }
    
    virtual MenuItemChoice const &get_choice(size_t index) const {
        return *_choices[index];
    }

    virtual MenuId get_id() const {
        return _item_id;
    }

private:

    char const *_label;
    MenuItemChoice const *const *_choices;
    size_t const _num_choices;
    MenuId const _item_id;
};


class Menu {

public:
    
    Menu(LiquidCrystal_I2C &lcd,
         MenuItem const *const *items,
         size_t num_items)
        : _lcd(lcd), _items(items), _num_items(num_items), _current_item_idx(0), _needs_redraw(true) {

        for(size_t i = 0; i < _num_items; i++) {
            _selections[i] = items[i]->get_default_choice_index();
        }
    }
    
    const MenuItem *const *get_items() {
        return _items;
    }

    MenuItemChoice const &get_selection_for_item_index(size_t item_index) {
        MenuItem const &item = *_items[item_index];
        size_t selection = _selections[item_index];
        return item.get_choice(selection);
    }

    MenuItemChoice const &get_selection_for_item_id(MenuId id) {
        for(size_t i = 0; i < _num_items; i++) {
            MenuItem const &item = *_items[i];
            if (item.get_id() == id) {
                size_t selection = _selections[i];
                return item.get_choice(selection);
            }
        }
        tfp_printf("get_selection_for_item_id: Unable to find item ID %d; returning something insensible", id);
        MenuItem const *item = _items[0];
        return item->get_choice(item->get_default_choice_index());
    }
        
    void redraw(bool force=false) {
        if (!(_needs_redraw || force)) {
            return;
        }

        // For space-padding the lines
        char fmt_lcdline[16];
        snprintf(fmt_lcdline,
                 sizeof(fmt_lcdline) / sizeof(*fmt_lcdline),
                 "%%-%ds", Lcd_cols);
        
        const MenuItem &item = get_current_item();

        char line1_text[Lcd_cols + 1];
        char line2_text[Lcd_cols + 1];

        char const *const label = item.get_label();
        snprintf(line1_text,
                 sizeof(line1_text) / sizeof(*line1_text),
                 fmt_lcdline,
                 label);

        char const *const choice_text = get_selection_for_current_item().get_label();
        snprintf(line2_text,
                 sizeof(line2_text) / sizeof(*line2_text),
                 fmt_lcdline,
                 choice_text);

        _lcd.setCursor(0, 0);
        _lcd.print(line1_text);
        _lcd.setCursor(0, 1);
        _lcd.print(line2_text);

        _needs_redraw = false;
    }

    KeyState process_keys(Joypad &jp) {
        KeyState ks = jp.get_pressed();

        if (ks.key_right()) {
            MenuItem const &item = get_current_item();
            
            size_t index = _selections[_current_item_idx];
            index = (index + 1) % item.get_num_choices();
            _selections[_current_item_idx] = index;

            _needs_redraw = true;
        } else if (ks.key_left()) {
            MenuItem const &item = get_current_item();
            
            size_t index = _selections[_current_item_idx];
            if (index == 0) {
                index = item.get_num_choices() - 1;
            } else {
                index--;
            }
            _selections[_current_item_idx] = index;

            _needs_redraw = true;
        } else if (ks.key_up()) {
            if (_current_item_idx == 0) {
                _current_item_idx = _num_items - 1;
            } else {
                _current_item_idx--;
            }
            _needs_redraw = true;
        } else if (ks.key_down()) {
            _current_item_idx = (_current_item_idx + 1) % _num_items;
            _needs_redraw = true;
        }

        redraw();
        return ks;
    }

    const MenuItem &get_current_item() {
        return *_items[_current_item_idx];
    }

    MenuItemChoice const &get_selection_for_current_item() {
        return get_selection_for_item_index(_current_item_idx);
    }
        
private:
    
    LiquidCrystal_I2C &_lcd;
    MenuItem const *const *_items;
    size_t const _num_items;

    size_t _current_item_idx;
    size_t _selections[120]; // TODO: this needs to be of length _num_items

    bool _needs_redraw;
    static int const Lcd_rows = 2;
    static int const Lcd_cols = 16;

};

#endif
