#include "menu_builder.h"

#include <stdint.h>

#include "EEPROM.h"

#include "new.h"
#include "menu_manualcontrol.h"
#include "menu_valvecontrol.h"

/*
 * Set up the menu.
 *
 * The ArrayMenuItem, ArrayMenuItemChoice, and Menu classes all accept
 * references to objects and don't copy them.  We need to be sure that
 * the objects we pass them stay in scope for the lifetime of the
 * Menu, so we allocate them all here as module-level globals.
 */

MenuId const MenuItemIdManualControl = 0;
MenuId const MenuItemIdValveOpenTime = 1;
MenuId const MenuItemIdValveToShutterReleaseTime = 2;
MenuId const MenuItemIdShutterReleaseTimeReference = 3;
MenuId const MenuItemIdValveControl = 4;
int const MenuItemCount = 5;

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

static MenuItem *menu_items_ptrs[MenuItemCount];
static size_t menu_items_count = 0;

//
// Menu item: Manual control
//

static uint8_t menu_item_manual_control_buf[sizeof(ManualControlMenuItem)];

//
// Menu item: Valve control
//

static uint8_t menu_item_valve_control_buf[sizeof(ValveControlMenuItem)];

//
// Menu item: Valve open time
//

static uint8_t menu_item_valve_open_time_buf[sizeof(TimeMenuItem)];

static char const *const valve_open_time_label = "Valve open time";

// What options are available eg. 4 => 4ms, 8ms, 12ms, etc.
static unsigned long const valve_open_time_step_small = 5;
static unsigned long const valve_open_time_step = 10;
static unsigned long const valve_open_time_step_large = 25;

static unsigned long const valve_open_time_initial = 25;


//
// Menu item: Valve to shutter release time
//

static uint8_t menu_item_valve_shutter_time_buf[sizeof(TimeMenuItem)];

static char const *const valve_shutter_time_label = "Shut. rel after";

// Number of choices
static int const valve_shutter_time_num_choices = 50;

// What options are available eg. 4 => 4ms, 8ms, 12ms, etc.
static unsigned long const valve_shutter_time_step_small = 5;
static unsigned long const valve_shutter_time_step = 25;
static unsigned long const valve_shutter_time_step_large = 100;

static unsigned long const valve_shutter_time_initial = 250;

//
// Menu item: Valve to shutter release time reference
// (ie. release 30 ms after valve open or valve close?)
//

static uint8_t menu_item_valve_shutter_reference_buf[sizeof(ArrayMenuItem)];

static char const *const valve_shutter_reference_label = "Shutter time ref";

// Number of choices
static int const valve_shutter_reference_num_choices = 2;

// Buffer for the choice objects
static uint8_t valve_shutter_reference_choices_buf[sizeof(ArrayMenuItemChoice) * valve_shutter_reference_num_choices];

// Buffer for pointers to the choice objects in valve_shutter_reference_choices_buf
static ArrayMenuItemChoice const *valve_shutter_reference_choices_ptrs[valve_shutter_reference_num_choices];

static char const *const valve_shutter_reference_choice_labels[] = {
    "From valve open",
    "From valve close"
};

//
// Private helpers
//

static void add_array_menu_item(MenuId id,
                                void *menu_item_buf,
                                char const *label,
                                ArrayMenuItemChoice const *const *choices,
                                size_t num_choices) {

    ArrayMenuItem *items_ptr = static_cast<ArrayMenuItem *>((void *)menu_item_buf);
    
    menu_items_ptrs[menu_items_count] = new (items_ptr) ArrayMenuItem(id, label, choices, num_choices, 0);
    
    menu_items_count++;
}

static void add_valve_open_time_menu() {
    
    TimeMenuItem *buf_ptr = static_cast<TimeMenuItem *>((void *)&menu_item_valve_open_time_buf);
    
    menu_items_ptrs[menu_items_count] = new (buf_ptr) TimeMenuItem(MenuItemIdValveOpenTime,
                                                                   valve_open_time_label,
                                                                   valve_open_time_step_small,
                                                                   valve_open_time_step,
                                                                   valve_open_time_step_large,
                                                                   valve_open_time_initial);
    menu_items_count++;
}

static void add_valve_shutter_time_menu() {

    TimeMenuItem *buf_ptr = static_cast<TimeMenuItem *>((void *)&menu_item_valve_shutter_time_buf);
    
    menu_items_ptrs[menu_items_count] = new (buf_ptr) TimeMenuItem(MenuItemIdValveToShutterReleaseTime,
                                                                   valve_shutter_time_label,
                                                                   valve_shutter_time_step_small,
                                                                   valve_shutter_time_step,
                                                                   valve_shutter_time_step_large,
                                                                   valve_shutter_time_initial);
    menu_items_count++;
}

static void add_valve_shutter_reference_menu() {

    ArrayMenuItemChoice *choices_ptr = static_cast<ArrayMenuItemChoice *>((void *)&valve_shutter_reference_choices_buf);
    
    valve_shutter_reference_choices_ptrs[0] = new (&choices_ptr[0]) ArrayMenuItemChoice(MenuItemChoiceIdShutterReleasesAfterValveOpen, valve_shutter_reference_choice_labels[0]);

    valve_shutter_reference_choices_ptrs[1] = new (&choices_ptr[1]) ArrayMenuItemChoice(MenuItemChoiceIdShutterReleasesAfterValveClose, valve_shutter_reference_choice_labels[1]);

    add_array_menu_item(MenuItemIdShutterReleaseTimeReference,
                        (void *)&menu_item_valve_shutter_reference_buf,
                        valve_shutter_reference_label,
                        valve_shutter_reference_choices_ptrs,
                        valve_shutter_reference_num_choices);
}

static void add_manual_control_menu() {

    ManualControlMenuItem *buf_ptr = static_cast<ManualControlMenuItem *>((void *)&menu_item_manual_control_buf);

    menu_items_ptrs[menu_items_count] = new (buf_ptr) ManualControlMenuItem(MenuItemIdManualControl);
    menu_items_count++;
}

static void add_valve_control_menu() {

    ValveControlMenuItem *buf_ptr = static_cast<ValveControlMenuItem *>((void *)&menu_item_valve_control_buf);

    menu_items_ptrs[menu_items_count] = new (buf_ptr) ValveControlMenuItem(MenuItemIdValveControl);
    menu_items_count++;
}

//
// Public interface
//

Menu &build_menu(LiquidCrystal_I2C &lcd) {
    add_manual_control_menu();
    add_valve_control_menu();
    
    add_valve_open_time_menu();
    add_valve_shutter_time_menu();
    add_valve_shutter_reference_menu();
    
    
    Menu *menu_buf_ptr = static_cast<Menu *>((void *)&menu_buf);
    menu_ptr = new (menu_buf_ptr) Menu(lcd,
                                       (MenuItem **)menu_items_ptrs,
                                       menu_items_count);

    
    return *menu_ptr;
}

unsigned long get_valve_open_time_ms() {
    Menu &menu = *menu_ptr;
    TimeMenuItem &item = *((TimeMenuItem *)menu.get_item_by_id(MenuItemIdValveOpenTime));

    return item.get_time();
}

unsigned long get_valve_to_shutter_time_ms() {
    Menu &menu = *menu_ptr;
    TimeMenuItem &item = *((TimeMenuItem *)menu.get_item_by_id(MenuItemIdValveToShutterReleaseTime));
    
    return item.get_time();
}

MenuId get_valve_shutter_reference() {
    Menu &menu = *menu_ptr;
    ArrayMenuItem &item = *((ArrayMenuItem *)menu.get_item_by_id(MenuItemIdShutterReleaseTimeReference));
    ArrayMenuItemChoice const &choice = item.get_selected_choice();

    return choice.get_id();
}
