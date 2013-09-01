#ifndef MENUOPTIONS_H_
#define MENUOPTIONS_H_

#include "LiquidCrystal_I2C.h"

#include "menu.h"

extern MenuId const MenuItemIdValveOpenTime;
extern MenuId const MenuItemIdValveToShutterReleaseTime;
extern MenuId const MenuItemIdShutterReleaseTimeReference;
extern MenuId const MenuItemCount;

extern MenuId const MenuItemChoiceIdShutterReleasesAfterValveOpen;
extern MenuId const MenuItemChoiceIdShutterReleasesAfterValveClose;

Menu &build_menu(LiquidCrystal_I2C &lcd);

#endif
