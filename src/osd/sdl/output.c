//============================================================
//
//  output.c - Generic implementation of MAME output routines
//
//  Copyright Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//============================================================

#if !defined(SDLMAME_WIN32)

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

// MAME headers
#include "emu.h"

// MAMEOS headers
#include "output.h"



//============================================================
//  CONSTANTS
//============================================================

#define SDLMAME_OUTPUT  "/tmp/sdlmame_out"

/*
 * Using long/int should be sufficient on all
 * architectures.
 */


#ifdef PTR64
#define PID_FMT "%ld"
#define PID_CAST long
#else
#define PID_FMT "%d"
#define PID_CAST int
#endif

//============================================================
//  TYPEDEFS
//============================================================

//============================================================
//  PRIVATE VARIABLES
//============================================================

static FILE *output;

//============================================================
//  FUNCTION PROTOTYPES
//============================================================

static void sdloutput_exit(running_machine &machine);
static void notifier_callback(const char *outname, INT32 value, void *param);

//============================================================
//  osd_get_pid
//============================================================

PID_CAST osd_getpid(void)
{
	return (PID_CAST) getpid();
}

//============================================================
//  sdloutput_init
//============================================================

void sdloutput_init(running_machine &machine)
{
	int fildes;

	machine.add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(sdloutput_exit), &machine));

	fildes = open(SDLMAME_OUTPUT, O_RDWR | O_NONBLOCK);

	if (fildes < 0)
	{
		output = NULL;
		mame_printf_verbose("output: unable to open output notifier file %s\n", SDLMAME_OUTPUT);
	}
	else
	{
		output = fdopen(fildes, "w");

		mame_printf_verbose("output: opened output notifier file %s\n", SDLMAME_OUTPUT);
		fprintf(output, "MAME " PID_FMT " START %s\n", osd_getpid(), machine.system().name);
		fflush(output);
	}

	output_set_notifier(NULL, notifier_callback, NULL);
}


//============================================================
//  winoutput_exit
//============================================================

static void sdloutput_exit(running_machine &machine)
{
	if (output != NULL)
	{
		fprintf(output, "MAME " PID_FMT " STOP %s\n", osd_getpid(), machine.system().name);
		fflush(output);
		fclose(output);
		output = NULL;
		mame_printf_verbose("output: closed output notifier file\n");
	}
}

//============================================================
//  notifier_callback
//============================================================

static void notifier_callback(const char *outname, INT32 value, void *param)
{
	if (output != NULL)
	{
		fprintf(output, "OUT " PID_FMT " %s %d\n", osd_getpid(), outname, value);
		fflush(output);
	}
}

#else  /* SDLMAME_WIN32 */

#include "emucore.h"

//============================================================
//  Stub for win32
//============================================================

void sdloutput_init(running_machine &machine)
{
}

#endif  /* SDLMAME_WIN32 */
