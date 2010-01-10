/***************************************************************************

    watchdog.h

    Watchdog handling

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __WATCHDOG_H__
#define __WATCHDOG_H__


/* startup */
void watchdog_init(running_machine *machine);

/* reset the watchdog */
void watchdog_reset(running_machine *machine);

/* enable/disable the watchdog */
void watchdog_enable(running_machine *machine, int enable);


#endif	/* __WATCHDOG_H__ */
