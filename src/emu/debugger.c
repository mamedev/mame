/*********************************************************************

    debugger.c

    Front-end debugger interfaces.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#include "driver.h"
#include "debugger.h"
#include "debug/debugcpu.h"
#include "debug/debugcmd.h"
#include "debug/debugcmt.h"
#include "debug/debugcon.h"
#include "debug/express.h"
#include "debug/debugvw.h"
#include <ctype.h>



/***************************************************************************
    CENTRAL INITIALIZATION POINT
***************************************************************************/

/*-------------------------------------------------
    debugger_init - start up all subsections
-------------------------------------------------*/

void debugger_init(running_machine *machine)
{
	/* only if debugging is enabled */
	if (machine->debug_flags & DEBUG_FLAG_ENABLED)
	{
		debug_cpu_init(machine);
		debug_command_init(machine);
		debug_console_init(machine);
		debug_view_init(machine);
		debug_comment_init(machine);
		atexit(debug_cpu_flush_traces);
		add_logerror_callback(machine, debug_errorlog_write_line);
	}
}


/*-------------------------------------------------
    debugger_refresh_display - redraw the current
    video display
-------------------------------------------------*/

void debugger_refresh_display(running_machine *machine)
{
	video_frame_update(machine, TRUE);
}


