// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles
/*********************************************************************

    debugger.cpp

    Front-end debugger interfaces.

*********************************************************************/

#include "emu.h"
#include "debugger.h"
#include "osdepend.h"
#include "debug/debugcpu.h"
#include "debug/debugcmd.h"
#include "debug/debugcon.h"
#include "debug/debugvw.h"
#include "debug/srcdbg_provider.h"
#include <cctype>

/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

static running_machine *g_machine = nullptr;
static bool g_atexit_registered = false;


/*-------------------------------------------------
    debug_break - stop in the debugger at the next
    opportunity
-------------------------------------------------*/

void debugger_manager::debug_break()
{
	m_console->get_visible_cpu()->debug()->halt_on_next_instruction("Internal breakpoint\n");
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
	m_debug_info = load_debug_info(machine);
	m_commands = std::make_unique<debugger_commands>(machine, cpu(), console());

	g_machine = &machine;

	/* register an atexit handler if we haven't yet */
	if (!g_atexit_registered)
		atexit(debugger_flush_all_traces_on_abnormal_exit);
	g_atexit_registered = true;

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
    load_debug_info - load the source-level
    debugging information file if enabled
-------------------------------------------------*/

std::unique_ptr<srcdbg_provider_base> debugger_manager::load_debug_info(running_machine &machine)
{
	return srcdbg_provider_base::create_debug_info(machine);
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

void debugger_flush_all_traces_on_abnormal_exit()
{
	if(g_machine != nullptr)
	{
		g_machine->debugger().cpu().flush_traces();
	}
}
