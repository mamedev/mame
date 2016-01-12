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



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void debugger_exit(running_machine &machine);



/***************************************************************************
    CENTRAL INITIALIZATION POINT
***************************************************************************/

/*-------------------------------------------------
    debugger_init - start up all subsections
-------------------------------------------------*/

void debugger_init(running_machine &machine)
{
	/* only if debugging is enabled */
	if (machine.debug_flags & DEBUG_FLAG_ENABLED)
	{
		/* initialize the submodules */
		machine.m_debug_view = std::make_unique<debug_view_manager>(machine);
		debug_cpu_init(machine);
		debug_command_init(machine);
		debug_console_init(machine);

		/* allocate a new entry for our global list */
		machine.add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(debugger_exit), &machine));
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
}


/*-------------------------------------------------
    debugger_refresh_display - redraw the current
    video display
-------------------------------------------------*/

void debugger_refresh_display(running_machine &machine)
{
	machine.video().frame_update(true);
}


/*-------------------------------------------------
    debugger_exit - remove ourself from the
    global list of active machines for cleanup
-------------------------------------------------*/

static void debugger_exit(running_machine &machine)
{
	g_machine = nullptr;
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
