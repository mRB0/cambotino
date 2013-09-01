#include "menu_builder.h"

#include <stdint.h>

#include "new.h"

/*
 * Set up the menu.
 *
 * The ArrayMenuItem, MenuItemChoice, and Menu classes all accept
 * references to objects and don't copy them.  We need to be sure
 * that the objects we pass them stay in scope for the lifetime of
 * the Menu, so we allocate them all here as module-level globals.
 */

MenuId const MenuItemIdValveOpenTime = 0;
MenuId const MenuItemIdValveToShutterReleaseTime = 1;
MenuId const MenuItemIdShutterReleaseTimeReference = 2;
MenuId const MenuItemCount = 3;

MenuId const MenuItemChoiceIdShutterReleasesAfterValveOpen = 0;
MenuId const MenuItemChoiceIdShutterReleasesAfterValveClose = 1;

//
// Shared data and constants
//

static int const label_len = 17; // 16 for an LCD row, + 1 for null terminator

//
// Menu storage
//

static uint8_t menu_buf[sizeof(Menu)];
static Menu *menu_ptr;

//
// Menu items
//

static uint8_t menu_items_buf[sizeof(ArrayMenuItem) * MenuItemCount];
static ArrayMenuItem *menu_items_ptrs[MenuItemCount];
static size_t menu_items_count = 0;

//
// Menu item: Valve open time
//

static char const *const valve_open_time_label = "Valve open time";

// Number of choices
static int const valve_open_time_num_choices = 50;

// Buffer for the choice objects
static uint8_t valve_open_time_choices_buf[sizeof(MenuItemChoice) * valve_open_time_num_choices];

// Buffer for pointers to the choice objects in valve_open_time_choices_buf
static MenuItemChoice const *valve_open_time_choices_ptrs[valve_open_time_num_choices];

// Buffer for the choice labels
static char valve_open_time_choicelabels_buf[sizeof(char) * label_len * valve_open_time_num_choices];

// Gosh, some macros would really help here huh?

//
// Private helpers
//

static void add_menu_item(MenuId id,
                          char const *label,
                          MenuItemChoice const *const *choices,
                          size_t num_choices) {

    ArrayMenuItem *items_ptr = static_cast<ArrayMenuItem *>((void *)&menu_items_buf);
    
    menu_items_ptrs[menu_items_count] = new (&items_ptr[menu_items_count]) ArrayMenuItem(id, label, choices, num_choices);

    menu_items_count++;
}

static void make_valve_open_time_menu() {
    
    MenuItemChoice *choices_ptr = static_cast<MenuItemChoice *>((void *)&valve_open_time_choices_buf);

    char *choicelabels_buf_next = valve_open_time_choicelabels_buf;
    
    for(int i = 0; i < valve_open_time_num_choices; i++) {
        int time_ms = i + 1;
        snprintf(choicelabels_buf_next,
                 label_len,
                 "%d ms", time_ms);
        valve_open_time_choices_ptrs[i] = new (&choices_ptr[i]) MenuItemChoice(time_ms, choicelabels_buf_next);
        choicelabels_buf_next += label_len;
    }

    add_menu_item(MenuItemIdValveOpenTime,
                  valve_open_time_label,
                  valve_open_time_choices_ptrs,
                  valve_open_time_num_choices);
}

//
// Public interface
//

Menu &build_menu(LiquidCrystal_I2C &lcd) {
    make_valve_open_time_menu();
    
    Menu *menu_buf_ptr = static_cast<Menu *>((void *)&menu_buf);
    menu_ptr = new (menu_buf_ptr) Menu(lcd,
                                       (MenuItem *const *)menu_items_ptrs,
                                       menu_items_count);

    
    return *menu_ptr;
}

uint8_t get_valve_open_time_ms() {
    Menu &menu = *menu_ptr;
    MenuItemChoice const &choice = menu.get_selection_for_item_id(MenuItemIdValveOpenTime);

    return (uint8_t)choice.get_id();
}
