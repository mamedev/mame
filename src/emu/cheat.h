/*********************************************************************

    cheat.h

    Cheat system.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#pragma once

#ifndef __CHEAT_H__
#define __CHEAT_H__

#include "mamecore.h"

void cheat_init(running_machine *machine);

int cheat_menu(running_machine *machine, int selection);

void cheat_display_watches(running_machine *machine);

#endif	/* __CHEAT_H__ */
