// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles
/*********************************************************************

    debugger.c

    Front-end debugger interfaces.
*********************************************************************/

#include "emu.h"
#include "debugger.h"
#include "osdepend.h"
#include "debug/debugcpu.h"
#include "debug/debugcmd.h"
#include "debug/debugcon.h"
#include "debug/debugvw.h"
#include <ctype.h>

/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

static running_machine *g_machine = nullptr;
static int g_atexit_registered = FALSE;

/*-------------------------------------------------
    debugger_instruction_hook - CPU cores call
    this once per instruction from CPU cores
-------------------------------------------------*/

void debugger_instruction_hook(device_t *device, offs_t curpc)
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

void debugger_exception_hook(device_t *device, int exception)
{
	if ((device->machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
		device->debug()->exception_hook(exception);
}

/*-------------------------------------------------
    debugger_start_cpu_hook - the CPU execution
    system calls this hook before beginning
    execution for the given CPU
-------------------------------------------------*/

void debugger_start_cpu_hook(device_t *device, const attotime &endtime)
{
	if ((device->machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
		device->debug()->start_hook(endtime);
}


/*-------------------------------------------------
    debugger_stop_cpu_hook - the CPU execution
    system calls this hook when ending execution
    for the given CPU
-------------------------------------------------*/

void debugger_stop_cpu_hook(device_t *device)
{
	if ((device->machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
		device->debug()->stop_hook();
}


/*-------------------------------------------------
    debugger_interrupt_hook - the CPU execution
    system calls this hook when an interrupt is
    acknowledged
-------------------------------------------------*/

void debugger_interrupt_hook(device_t *device, int irqline)
{
	if ((device->machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
		device->debug()->interrupt_hook(irqline);
}

/*-------------------------------------------------
    debug_break - stop in the debugger at the next
    opportunity
-------------------------------------------------*/

void debugger_manager::debug_break()
{
	if ((m_machine.debug_flags & DEBUG_FLAG_ENABLED) != 0)
		m_cpu->get_visible_cpu()->debug()->halt_on_next_instruction("Internal breakpoint\n");
}


/*-------------------------------------------------
    within_instruction_hook - call this to
    determine if the debugger is currently halted
    within the instruction hook
-------------------------------------------------*/

bool debugger_manager::within_instruction_hook()
{
	if ((m_machine.debug_flags & DEBUG_FLAG_ENABLED) != 0)
		return m_cpu->within_instruction_hook();
	return false;
}


//**************************************************************************
//  DEBUGGER MANAGER
//**************************************************************************

//-------------------------------------------------
//  debugger_manager - constructor
//-------------------------------------------------

debugger_manager::debugger_manager(running_machine &machine)
	: m_machine(machine)
{
	/* initialize the submodules */
	m_cpu = std::make_unique<debugger_cpu>(machine);
	m_console = std::make_unique<debugger_console>(machine);
    m_commands = std::make_unique<debugger_commands>(machine, cpu(), console());

	g_machine = &machine;

	/* register an atexit handler if we haven't yet */
	if (!g_atexit_registered)
		atexit(debugger_flush_all_traces_on_abnormal_exit);
	g_atexit_registered = TRUE;

	/* initialize osd debugger features */
	machine.osd().init_debugger();
}

/*-------------------------------------------------
//  debugger_manager - destructor
-------------------------------------------------*/

debugger_manager::~debugger_manager()
{
	g_machine = nullptr;
}

/*-------------------------------------------------
    refresh_display - redraw the current
    video display
-------------------------------------------------*/

void debugger_manager::refresh_display()
{
	machine().video().frame_update(true);
}


/*-------------------------------------------------
    debugger_flush_all_traces_on_abnormal_exit -
    flush any traces in the event of an aborted
    execution
-------------------------------------------------*/

void debugger_flush_all_traces_on_abnormal_exit(void)
{
	if(g_machine != nullptr)
	{
		g_machine->debugger().cpu().flush_traces();
	}
}
