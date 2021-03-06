/***************************************************************************

    ui/mainmenu.h

    Internal MAME menus for the user interface.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __UI_MAINMENU_H__
#define __UI_MAINMENU_H__

#include "crsshair.h"
#include "drivenum.h"

class ui_menu_main : public ui_menu {
public:
	ui_menu_main(running_machine &machine, render_container *container);
	virtual ~ui_menu_main();
	virtual void populate();
	virtual void handle();

private:
	enum {
		INPUT_GROUPS,
		INPUT_SPECIFIC,
#ifdef USE_AUTOFIRE
		AUTOFIRE,
#endif /* USE_AUTOFIRE */
#ifdef USE_CUSTOM_BUTTON
		CUSTOM_BUTTON,
#endif /* USE_CUSTOM_BUTTON */
		SETTINGS_DIP_SWITCHES,
		SETTINGS_DRIVER_CONFIG,
		ANALOG,
		BOOKKEEPING,
		GAME_INFO,
		IMAGE_MENU_IMAGE_INFO,
		IMAGE_MENU_FILE_MANAGER,
		TAPE_CONTROL,
		SLOT_DEVICES,
		NETWORK_DEVICES,
		KEYBOARD_MODE,
		SLIDERS,
		VIDEO_TARGETS,
		VIDEO_OPTIONS,
#ifdef USE_SCALE_EFFECTS
		SCALE_EFFECT,
#endif /* USE_SCALE_EFFECTS */
		CROSSHAIR,
		CHEAT,
#ifdef CMD_LIST
		COMMAND,
#endif /* CMD_LIST */
		SELECT_GAME,
		BIOS_SELECTION,
		BARCODE_READ,
	};
};

#endif  /* __UI_MAINMENU_H__ */
