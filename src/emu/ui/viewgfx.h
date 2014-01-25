/***************************************************************************

    ui/viewgfx.h

    Internal graphics viewer.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __UI_VIEWGFX_H__
#define __UI_VIEWGFX_H__



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

// initialization
void ui_gfx_init(running_machine &machine);

// returns 'true' if the internal graphics viewer has relevance
bool ui_gfx_is_relevant(running_machine &machine);

// master handler
UINT32 ui_gfx_ui_handler(running_machine &machine, render_container *container, UINT32 state);


#endif  /* __UI_VIEWGFX_H__ */
