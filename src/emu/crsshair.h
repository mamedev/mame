/***************************************************************************

    crsshair.h

    Crosshair handling.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __CRSSHAIR_H__
#define __CRSSHAIR_H__


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define CROSSHAIR_SCREEN_NONE	((const device_config *) 0)
#define CROSSHAIR_SCREEN_ALL	((const device_config *) ~0)



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* initializes the crosshair system */
void crosshair_init(running_machine *machine);

/* draws crosshair(s) in a given screen, if neccessary */
void crosshair_render(const device_config *screen);

/* toggles crosshair visibility */
void crosshair_toggle(running_machine *machine);

/* sets the screen(s) for a given player's crosshair */
void crosshair_set_screen(running_machine *machine, int player, const device_config *screen);



#endif	/* __CRSSHAIR_H__ */
