#ifndef MENUOPTIONS_H_
#define MENUOPTIONS_H_

#include "LiquidCrystal_I2C.h"

#include "menu.h"

extern MenuId const MenuItemIdManualControl;
extern MenuId const MenuItemIdValveOpenTime;
extern MenuId const MenuItemIdValveToShutterReleaseTime;
extern MenuId const MenuItemIdShutterReleaseTimeReference;
extern MenuId const MenuItemIdValveControl;
extern int const MenuItemCount;

extern MenuId const MenuItemChoiceIdShutterReleasesAfterValveOpen;
extern MenuId const MenuItemChoiceIdShutterReleasesAfterValveClose;

Menu &build_menu(LiquidCrystal_I2C &lcd);

unsigned long get_valve_open_time_ms();
unsigned long get_valve_to_shutter_time_ms();
MenuId get_valve_shutter_reference();

#endif
