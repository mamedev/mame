/****************************************************************************

    debugger.h

    General debugging interfaces

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

****************************************************************************/

#pragma once

#ifndef __DEBUGGER_H__
#define __DEBUGGER_H__

#include "mame.h"
#include "deprecat.h"
#include "debug/debugcpu.h"


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* ----- core debugger functions ----- */

/* initialize the debugger */
void debugger_init(running_machine *machine);

/* redraw the current video display */
void debugger_refresh_display(running_machine *machine);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    debugger_instruction_hook - CPU cores call
    this once per instruction from CPU cores
-------------------------------------------------*/

INLINE void debugger_instruction_hook(running_machine *machine, offs_t curpc)
{
	if ((machine->debug_flags & DEBUG_FLAG_CALL_HOOK) != 0)
		debug_cpu_instruction_hook(machine, curpc);
}


/*-------------------------------------------------
    debugger_start_cpu_hook - the CPU execution
    system calls this hook before beginning
    execution for the given CPU
-------------------------------------------------*/

INLINE void debugger_start_cpu_hook(running_machine *machine, int cpunum, attotime endtime)
{
	if ((machine->debug_flags & DEBUG_FLAG_ENABLED) != 0)
		debug_cpu_start_hook(machine, cpunum, endtime);
}


/*-------------------------------------------------
    debugger_stop_cpu_hook - the CPU execution
    system calls this hook when ending execution
    for the given CPU
-------------------------------------------------*/

INLINE void debugger_stop_cpu_hook(running_machine *machine, int cpunum)
{
	if ((machine->debug_flags & DEBUG_FLAG_ENABLED) != 0)
		debug_cpu_stop_hook(machine, cpunum);
}


/*-------------------------------------------------
    debugger_break - stop in the debugger at the
    next opportunity
-------------------------------------------------*/

INLINE void debugger_break(running_machine *machine)
{
	if ((machine->debug_flags & DEBUG_FLAG_ENABLED) != 0)
		debug_cpu_halt_on_next_instruction(machine, "Internal breakpoint\n");
}


/*-------------------------------------------------
    debugger_within_instruction_hook - call this
    to determine if the debugger is currently
    halted within the instruction hook
-------------------------------------------------*/

INLINE int debugger_within_instruction_hook(running_machine *machine)
{
	if ((machine->debug_flags & DEBUG_FLAG_ENABLED) != 0)
		return debug_cpu_within_instruction_hook(machine);
	return FALSE;
}


#endif	/* __DEBUGGER_H__ */
