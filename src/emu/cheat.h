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



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/


/* ----- core system management ----- */

/* initialize the cheat system, loading any cheat files */
void cheat_init(running_machine *machine);

/* re-initialize the cheat system, reloading any cheat files */
void cheat_reload(running_machine *machine);



/* ----- cheat UI helpers ----- */

/* render any text overlays */
void cheat_render_text(running_machine *machine);

/* return data about the next menu entry, or the first entry if previous == NULL */
void *cheat_get_next_menu_entry(running_machine *machine, void *previous, const char **description, const char **state, UINT32 *flags);

/* activate a oneshot cheat */
int cheat_activate(running_machine *machine, void *entry);

/* select the default menu state */
int cheat_select_default_state(running_machine *machine, void *entry);

/* select the previous menu state */
int cheat_select_previous_state(running_machine *machine, void *entry);

/* select the next menu state */
int cheat_select_next_state(running_machine *machine, void *entry);

/* return the displayable comment of the current cheat */
astring *cheat_get_comment(void *entry);


#endif	/* __CHEAT_H__ */
