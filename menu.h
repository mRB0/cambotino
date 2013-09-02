#ifndef MENU_H_
#define MENU_H_

#include <stdint.h>

#include "LiquidCrystal_I2C.h"

#include "joypad.h"
#include "printf.h"

typedef uint16_t MenuId;
typedef uint32_t SelectionValue;

/////////////////////////////////////////////////////////////////////////

class MenuItem {

public:

    virtual char const *get_label() const = 0;
    virtual char const *get_selection_label() const = 0;
    
    virtual MenuId get_id() const {
        return 0;
    }

    virtual SelectionValue get_selection_value() const = 0;
    
    virtual bool process_keypress(KeyState const &keys, bool *redraw) {
        *redraw = false;
        return false;
    }
};

/////////////////////////////////////////////////////////////////////////

class ArrayMenuItemChoice {

public:
    
    ArrayMenuItemChoice(MenuId id, char const *label)
        : _id(id), _label(label) {}

    char const *get_label() const { return _label; }
    MenuId get_id() const { return _id; }
    
    MenuId const _id;
    char const *const _label;
};

/////////////////////////////////////////////////////////////////////////

class ArrayMenuItem : public MenuItem {

public:

ArrayMenuItem(MenuId id, char const *label, ArrayMenuItemChoice const *const *choices, size_t num_choices, size_t initial_selection)
        : _label(label), _choices(choices), _num_choices(num_choices), _item_id(id), _selected(initial_selection) {
    }
    
    virtual char const *get_label() const {
        return _label;
    }
    
    virtual size_t get_num_choices() const {
        return _num_choices;
    }
    
    virtual char const *get_selection_label() const {
        return get_choice(_selected).get_label();
    }

    ArrayMenuItemChoice const &get_choice(size_t index) const {
        return *_choices[index];
    }

    ArrayMenuItemChoice const &get_selected_choice() const {
        return get_choice(_selected);
    }

    virtual MenuId get_id() const {
        return _item_id;
    }

    virtual SelectionValue get_selection_value() const {
        return get_choice(_selected).get_id();
    }

    virtual bool process_keypress(KeyState const &keys, bool *redraw) {
        if (keys.key_right()) {
            _selected = (_selected + 1) % get_num_choices();

            *redraw = true;
        } else if (keys.key_left()) {
            if (_selected == 0) {
                _selected = get_num_choices() - 1;
            } else {
                _selected--;
            }
            *redraw = true;
        }
        return *redraw;
    }

private:

    char const *_label;
    ArrayMenuItemChoice const *const *_choices;
    size_t const _num_choices;
    MenuId const _item_id;
    
    size_t _selected;
};

/////////////////////////////////////////////////////////////////////////

class Menu {

public:
    
    Menu(LiquidCrystal_I2C &lcd,
         MenuItem *const *items,
         size_t num_items)
        : _lcd(lcd), _items(items), _num_items(num_items), _current_item_idx(0), _needs_redraw(true) {
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
        
        MenuItem &item = get_current_item();

        char line1_text[Lcd_cols + 1];
        char line2_text[Lcd_cols + 1];

        char const *const label = item.get_label();
        snprintf(line1_text,
                 sizeof(line1_text) / sizeof(*line1_text),
                 fmt_lcdline,
                 label);

        char const *const choice_text = item.get_selection_label();
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

        if (ks.key_up()) {
            if (_current_item_idx == 0) {
                _current_item_idx = _num_items - 1;
            } else {
                _current_item_idx--;
            }
            _needs_redraw = true;
        } else if (ks.key_down()) {
            _current_item_idx = (_current_item_idx + 1) % _num_items;
            _needs_redraw = true;
        } else {
            bool redraw = false;
            get_current_item().process_keypress(ks, &redraw);
            _needs_redraw |= redraw;
        }

        redraw();
        return ks;
    }

    MenuItem &get_current_item() {
        return *_items[_current_item_idx];
    }

    MenuItem *get_item_by_id(MenuId id) {
        for(size_t i = 0; i < _num_items; i++) {
            MenuItem *item = _items[i];
            if (item->get_id() == id) {
                return item;
            }
        }
        return NULL;
    }
    
    MenuItem *const *get_items() {
        return _items;
    }

private:
    
    LiquidCrystal_I2C &_lcd;
    MenuItem *const *_items;
    size_t const _num_items;

    size_t _current_item_idx;

    bool _needs_redraw;
    static int const Lcd_rows = 2;
    static int const Lcd_cols = 16;

};

#endif
