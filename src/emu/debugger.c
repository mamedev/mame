/*********************************************************************

    debugger.c

    Front-end debugger interfaces.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#include "emu.h"
#include "debugger.h"
#include "osdepend.h"
#include "debug/debugcpu.h"
#include "debug/debugcmd.h"
#include "debug/debugcon.h"
#include "debug/express.h"
#include "debug/debugvw.h"
#include "debugint/debugint.h"
#include <ctype.h>



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct machine_entry
{
	machine_entry *     next;
	running_machine *   machine;
};



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

static machine_entry *machine_list;
static int atexit_registered;



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
		machine_entry *entry;

		/* initialize the submodules */
		machine.m_debug_view = auto_alloc(machine, debug_view_manager(machine));
		debug_cpu_init(machine);
		debug_command_init(machine);
		debug_console_init(machine);

		/* always initialize the internal render debugger */
		debugint_init(machine);

		/* allocate a new entry for our global list */
		machine.add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(debugger_exit), &machine));
		entry = global_alloc(machine_entry);
		entry->next = machine_list;
		entry->machine = &machine;
		machine_list = entry;

		/* register an atexit handler if we haven't yet */
		if (!atexit_registered)
			atexit(debugger_flush_all_traces_on_abnormal_exit);
		atexit_registered = TRUE;

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
	machine_entry **entryptr;

	/* remove this machine from the list; it came down cleanly */
	for (entryptr = &machine_list; *entryptr != NULL; entryptr = &(*entryptr)->next)
		if ((*entryptr)->machine == &machine)
		{
			machine_entry *deleteme = *entryptr;
			*entryptr = deleteme->next;
			global_free(deleteme);
			break;
		}
}


/*-------------------------------------------------
    debugger_flush_all_traces_on_abnormal_exit -
    flush any traces in the event of an aborted
    execution
-------------------------------------------------*/

void debugger_flush_all_traces_on_abnormal_exit(void)
{
	/* clear out the machine list and flush traces on each one */
	while (machine_list != NULL)
	{
		machine_entry *deleteme = machine_list;
		debug_cpu_flush_traces(*deleteme->machine);
		machine_list = deleteme->next;
		global_free(deleteme);
	}
}
