#ifndef MENU_H_
#define MENU_H_

#include <stdint.h>

#include "LiquidCrystal_I2C.h"

#include "joypad.h"
#include "printf.h"

typedef uint16_t MenuId;

/////////////////////////////////////////////////////////////////////////

class MenuItem {

public:

    virtual char const *get_label(char *label_buf, size_t buflen) const = 0;
    virtual char const *get_selection_label(char *label_buf, size_t buflen) const = 0;
    
    virtual MenuId get_id() const {
        return 0;
    }

    virtual bool process_keys(KeyState const &pressed_keys, KeyState const &held_keys) {
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
    
    virtual char const *get_label(char *label_buf, size_t buflen) const {
        return _label;
    }
    
    virtual size_t get_num_choices() const {
        return _num_choices;
    }
    
    virtual char const *get_selection_label(char *label_buf, size_t buflen) const {
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

    virtual bool process_keys(KeyState const &pressed_keys, KeyState const &held_keys) {
        bool processed = false;
        if (pressed_keys.key_right()) {
            _selected = (_selected + 1) % get_num_choices();
            processed = true;
        } else if (pressed_keys.key_left()) {
            if (_selected == 0) {
                _selected = get_num_choices() - 1;
            } else {
                _selected--;
            }
            processed = true;
        }
        return processed;
    }

private:

    char const *_label;
    ArrayMenuItemChoice const *const *_choices;
    size_t const _num_choices;
    MenuId const _item_id;
    
    size_t _selected;
};

/////////////////////////////////////////////////////////////////////////

class TimeMenuItem : public MenuItem {

public:

    TimeMenuItem(MenuId id,
                 char const *label,
                 unsigned long time_step_small,
                 unsigned long time_step,
                 unsigned long time_step_large,
                 unsigned long initial_time)
        : _id(id),
          _label(label),
          _time_step_small(time_step_small),
          _time_step(time_step),
          _time_step_large(time_step_large),
          _time(initial_time) {}

    virtual char const *get_label(char *label_buf, size_t buflen) const { return _label; }
    
    virtual char const *get_selection_label(char *label_buf, size_t buflen) const {
        snprintf(label_buf,
                 buflen,
                 "%lu ms", _time);

        return label_buf;
    }
    
    virtual MenuId get_id() const {
        return _id;
    }

    virtual unsigned long get_time() const {
        return _time;
    }
    
    virtual bool process_keys(KeyState const &pressed_keys, KeyState const &held_keys) {
        unsigned long step = _time_step;

        if (held_keys.key_a()) {
            step = _time_step_large;
        }

        if (held_keys.key_b()) {
            step = _time_step_small;
        }
        
        if (pressed_keys.key_left()) {
            if (_time > step) {
                _time -= step;
                return true;
            }
        }

        if (pressed_keys.key_right()) {
            _time += step;
            return true;
        }
        
        return false;
    }
    
private:

    MenuId const _id;
    char const *_label;

    unsigned long const _time_step_small;
    unsigned long const _time_step;
    unsigned long const _time_step_large;

    unsigned long _time;
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

        char label_buf[Lcd_cols + 1];
        char const *label;

        label = item.get_label((char *)&label_buf,
                               sizeof(label_buf) / sizeof(*label_buf));
        if (!label) {
            label = label_buf;
        }
        snprintf(line1_text,
                 sizeof(line1_text) / sizeof(*line1_text),
                 fmt_lcdline,
                 label);


        label = item.get_selection_label((char *)&label_buf,
                               sizeof(label_buf) / sizeof(*label_buf));
        if (!label) {
            label = label_buf;
        }

        snprintf(line2_text,
                 sizeof(line2_text) / sizeof(*line2_text),
                 fmt_lcdline,
                 label);

        _lcd.setCursor(0, 0);
        _lcd.print(line1_text);
        _lcd.setCursor(0, 1);
        _lcd.print(line2_text);

        _needs_redraw = false;
    }

    KeyState process_keys(Joypad &jp) {
        KeyState ks = jp.get_pressed();
        KeyState heldkeys = jp.get_held();
        
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
            _needs_redraw |= get_current_item().process_keys(ks, heldkeys);
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
