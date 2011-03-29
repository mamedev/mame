/*********************************************************************

    debugint.c

    Internal debugger frontend using render interface.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#pragma once

#ifndef __DEBUGINT_H__
#define __DEBUGINT_H__


/***************************************************************************
    CONSTANTS
***************************************************************************/


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* initialize the internal debugger */
void debugint_init(running_machine &machine);

/* process events for internal debugger */
void debugint_wait_for_debugger(device_t &device, bool firststop);

/* update the  internal debugger during a game */
void debugint_update_during_game(running_machine &machine);

#endif
