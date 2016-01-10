// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*********************************************************************

    bookkeeping.h

    Bookkeeping simple machine functions.

*********************************************************************/

#pragma once

#ifndef __BOOKKEEPING_H__
#define __BOOKKEEPING_H__



/***************************************************************************
    CONSTANTS
***************************************************************************/

/* total # of coin counters */
#define COIN_COUNTERS           8

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/


/* ----- initialization ----- */

/* set up all the common systems */
void generic_machine_init(running_machine &machine);



/* ----- tickets ----- */

/* return the number of tickets dispensed */
int get_dispensed_tickets(running_machine &machine);

/* increment the number of dispensed tickets */
void increment_dispensed_tickets(running_machine &machine, int delta);



/* ----- coin counters ----- */

/* write to a particular coin counter (clocks on active high edge) */
void coin_counter_w(running_machine &machine, int num, int on);

/* return the coin count for a given coin */
int coin_counter_get_count(running_machine &machine, int num);

/* enable/disable coin lockout for a particular coin */
void coin_lockout_w(running_machine &machine, int num, int on);

/* return current lockout state for a particular coin */
int coin_lockout_get_state(running_machine &machine, int num);

/* enable/disable global coin lockout */
void coin_lockout_global_w(running_machine &machine, int on);

#endif  /* __BOOKKEEPING_H__ */
