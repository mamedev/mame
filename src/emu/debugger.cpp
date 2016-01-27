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
	debug_cpu_init(machine);
	debug_command_init(machine);

	g_machine = &machine;

	/* register an atexit handler if we haven't yet */
	if (!g_atexit_registered)
		atexit(debugger_flush_all_traces_on_abnormal_exit);
	g_atexit_registered = TRUE;

	/* listen in on the errorlog */
	machine.add_logerror_callback(debug_errorlog_write_line);

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

void debugger_manager::initialize()
{
	/* only if debugging is enabled */
	if (machine().debug_flags & DEBUG_FLAG_ENABLED)
	{
		debug_console_init(machine());
	}
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
	if(g_machine!=nullptr)
	{
		debug_cpu_flush_traces(*g_machine);
	}
}
