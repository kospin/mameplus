/*********************************************************************

    cheat.h

    Cheat system.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#pragma once

#ifndef __CHEAT_H__
#define __CHEAT_H__

#ifdef USE_HISCORE
extern int he_did_cheat;
#endif /* USE_HISCORE */

#include "mamecore.h"

void cheat_init(running_machine *machine);

UINT32 cheat_menu(running_machine *machine, UINT32 state);

void cheat_display_watches(void);

#endif	/* __CHEAT_H__ */
