// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
//============================================================
//
//  output.c - Generic implementation of MAME output routines
//
//============================================================

#if !defined(SDLMAME_WIN32)

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <inttypes.h>

// MAME headers
#include "emu.h"

// MAMEOS headers
#include "osdsdl.h"



//============================================================
//  CONSTANTS
//============================================================

#define SDLMAME_OUTPUT  "/tmp/sdlmame_out"

/*
 * Using long/int should be sufficient on all
 * architectures.
 */


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

static void notifier_callback(const char *outname, INT32 value, void *param);

//============================================================
//  osd_get_pid
//============================================================

intptr_t osd_getpid(void)
{
	return intptr_t(getpid());
}

//============================================================
//  output_init
//============================================================

bool sdl_osd_interface::output_init()
{
	int fildes;

	fildes = open(SDLMAME_OUTPUT, O_RDWR | O_NONBLOCK);

	if (fildes < 0)
	{
		output = NULL;
		osd_printf_verbose("output: unable to open output notifier file %s\n", SDLMAME_OUTPUT);
	}
	else
	{
		output = fdopen(fildes, "w");

		osd_printf_verbose("output: opened output notifier file %s\n", SDLMAME_OUTPUT);
		fprintf(output, "MAME %" PRIxPTR " START %s\n", osd_getpid(), this->machine().system().name);
		fflush(output);
	}

	machine().output().set_notifier(NULL, notifier_callback, NULL);
	return true;
}


//============================================================
//  winoutput_exit
//============================================================

void sdl_osd_interface::output_exit()
{
	if (output != NULL)
	{
		fprintf(output, "MAME %" PRIxPTR " STOP %s\n", osd_getpid(), machine().system().name);
		fflush(output);
		fclose(output);
		output = NULL;
		osd_printf_verbose("output: closed output notifier file\n");
	}
}

//============================================================
//  notifier_callback
//============================================================

static void notifier_callback(const char *outname, INT32 value, void *param)
{
	if (output != NULL)
	{
		fprintf(output, "OUT %" PRIxPTR " %s %d\n", osd_getpid(), outname, value);
		fflush(output);
	}
}

#else  /* SDLMAME_WIN32 */

#include "emu.h"
#include "osdsdl.h"
#include "emucore.h"

//============================================================
//  Stub for win32
//============================================================

bool sdl_osd_interface::output_init()
{
	return true;
}

void sdl_osd_interface::output_exit()
{
}

#endif  /* SDLMAME_WIN32 */
