/****************************************************************************

    debugger.h

    General debugging interfaces

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

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
void debugger_init(running_machine *machine);

/* redraw the current video display */
void debugger_refresh_display(running_machine *machine);

/* OSD can call this to safely flush all traces in the event of a crash */
void debugger_flush_all_traces_on_abnormal_exit(void);



/***************************************************************************
    CPU CORE INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    debugger_instruction_hook - CPU cores call
    this once per instruction from CPU cores
-------------------------------------------------*/

INLINE void debugger_instruction_hook(running_device *device, offs_t curpc)
{
	if ((device->machine->debug_flags & DEBUG_FLAG_CALL_HOOK) != 0)
		debug_cpu_instruction_hook(device, curpc);
}


/*-------------------------------------------------
    debugger_exception_hook - CPU cores call this
    anytime an exception is generated
-------------------------------------------------*/

INLINE void debugger_exception_hook(running_device *device, int exception)
{
	if ((device->machine->debug_flags & DEBUG_FLAG_ENABLED) != 0)
		debug_cpu_exception_hook(device, exception);
}



/***************************************************************************
    CPU EXECUTION SYSTEM INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    debugger_start_cpu_hook - the CPU execution
    system calls this hook before beginning
    execution for the given CPU
-------------------------------------------------*/

INLINE void debugger_start_cpu_hook(running_device *device, attotime endtime)
{
	if ((device->machine->debug_flags & DEBUG_FLAG_ENABLED) != 0)
		debug_cpu_start_hook(device, endtime);
}


/*-------------------------------------------------
    debugger_stop_cpu_hook - the CPU execution
    system calls this hook when ending execution
    for the given CPU
-------------------------------------------------*/

INLINE void debugger_stop_cpu_hook(running_device *device)
{
	if ((device->machine->debug_flags & DEBUG_FLAG_ENABLED) != 0)
		debug_cpu_stop_hook(device);
}


/*-------------------------------------------------
    debugger_interrupt_hook - the CPU execution
    system calls this hook when an interrupt is
    acknowledged
-------------------------------------------------*/

INLINE void debugger_interrupt_hook(running_device *device, int irqline)
{
	if ((device->machine->debug_flags & DEBUG_FLAG_ENABLED) != 0)
		debug_cpu_interrupt_hook(device, irqline);
}



/***************************************************************************
    GENERAL INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    debugger_break - stop in the debugger at the
    next opportunity
-------------------------------------------------*/

INLINE void debugger_break(running_machine *machine)
{
	if ((machine->debug_flags & DEBUG_FLAG_ENABLED) != 0)
		debug_cpu_halt_on_next_instruction(debug_cpu_get_visible_cpu(machine), "Internal breakpoint\n");
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
