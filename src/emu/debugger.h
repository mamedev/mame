// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles
/****************************************************************************

    debugger.h

    General debugging interfaces
****************************************************************************/

#pragma once

#ifndef __DEBUGGER_H__
#define __DEBUGGER_H__

#include "debug/debugcpu.h"


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* ----- core debugger functions ----- */

/* initialize the debugger */
void debugger_init(running_machine &machine);

/* redraw the current video display */
void debugger_refresh_display(running_machine &machine);

/* OSD can call this to safely flush all traces in the event of a crash */
void debugger_flush_all_traces_on_abnormal_exit(void);



/***************************************************************************
    CPU CORE INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    debugger_instruction_hook - CPU cores call
    this once per instruction from CPU cores
-------------------------------------------------*/

static inline void debugger_instruction_hook(device_t *device, offs_t curpc)
{
#ifndef MAME_DEBUG_FAST
	if ((device->machine().debug_flags & DEBUG_FLAG_CALL_HOOK) != 0)
		device->debug()->instruction_hook(curpc);
#endif
}


/*-------------------------------------------------
    debugger_exception_hook - CPU cores call this
    anytime an exception is generated
-------------------------------------------------*/

static inline void debugger_exception_hook(device_t *device, int exception)
{
	if ((device->machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
		device->debug()->exception_hook(exception);
}



/***************************************************************************
    CPU EXECUTION SYSTEM INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    debugger_start_cpu_hook - the CPU execution
    system calls this hook before beginning
    execution for the given CPU
-------------------------------------------------*/

static inline void debugger_start_cpu_hook(device_t *device, const attotime &endtime)
{
	if ((device->machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
		device->debug()->start_hook(endtime);
}


/*-------------------------------------------------
    debugger_stop_cpu_hook - the CPU execution
    system calls this hook when ending execution
    for the given CPU
-------------------------------------------------*/

static inline void debugger_stop_cpu_hook(device_t *device)
{
	if ((device->machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
		device->debug()->stop_hook();
}


/*-------------------------------------------------
    debugger_interrupt_hook - the CPU execution
    system calls this hook when an interrupt is
    acknowledged
-------------------------------------------------*/

static inline void debugger_interrupt_hook(device_t *device, int irqline)
{
	if ((device->machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
		device->debug()->interrupt_hook(irqline);
}



/***************************************************************************
    GENERAL INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    debugger_break - stop in the debugger at the
    next opportunity
-------------------------------------------------*/

static inline void debugger_break(running_machine &machine)
{
	if ((machine.debug_flags & DEBUG_FLAG_ENABLED) != 0)
		debug_cpu_get_visible_cpu(machine)->debug()->halt_on_next_instruction("Internal breakpoint\n");
}


/*-------------------------------------------------
    debugger_within_instruction_hook - call this
    to determine if the debugger is currently
    halted within the instruction hook
-------------------------------------------------*/

static inline int debugger_within_instruction_hook(running_machine &machine)
{
	if ((machine.debug_flags & DEBUG_FLAG_ENABLED) != 0)
		return debug_cpu_within_instruction_hook(machine);
	return FALSE;
}


#endif  /* __DEBUGGER_H__ */
