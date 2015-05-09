// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
//============================================================
//
//  sdlos_*.c - OS specific low level code
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

// standard sdl header
#include "sdlinc.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/stat.h>

#define INCL_DOS
#include <os2.h>

// MAME headers
#include "osdcore.h"
#include "osdlib.h"

//============================================================
//  osd_getenv
//============================================================

const char *osd_getenv(const char *name)
{
	return getenv(name);
}

//============================================================
//  osd_setenv
//============================================================

int osd_setenv(const char *name, const char *value, int overwrite)
{
	return setenv(name, value, overwrite);
}

//============================================================
//  osd_process_kill
//============================================================

void osd_process_kill(void)
{
	PPIB ppib;

	DosGetInfoBlocks(NULL, &ppib);
	DosKillProcess(DKP_PROCESSTREE, ppib->pib_ulpid);
}

//============================================================
//  osd_num_processors
//============================================================

int osd_get_num_processors(void)
{
	ULONG numprocs = 1;

	DosQuerySysInfo(QSV_NUMPROCESSORS, QSV_NUMPROCESSORS, &numprocs, sizeof(numprocs));

	return numprocs;
}

//============================================================
//  osd_malloc
//============================================================

void *osd_malloc(size_t size)
{
#ifndef MALLOC_DEBUG
	return malloc(size);
#else
#error "MALLOC_DEBUG not yet supported"
#endif
}


//============================================================
//  osd_malloc_array
//============================================================

void *osd_malloc_array(size_t size)
{
#ifndef MALLOC_DEBUG
	return malloc(size);
#else
#error "MALLOC_DEBUG not yet supported"
#endif
}


//============================================================
//  osd_free
//============================================================

void osd_free(void *ptr)
{
#ifndef MALLOC_DEBUG
	free(ptr);
#else
#error "MALLOC_DEBUG not yet supported"
#endif
}


//============================================================
//  osd_alloc_executable
//
//  allocates "size" bytes of executable memory.  this must take
//  things like NX support into account.
//============================================================

void *osd_alloc_executable(size_t size)
{
	void *p;

	DosAllocMem( &p, size, fALLOC );
	return p;
}

//============================================================
//  osd_free_executable
//
//  frees memory allocated with osd_alloc_executable
//============================================================

void osd_free_executable(void *ptr, size_t size)
{
	DosFreeMem( ptr );
}

//============================================================
//  osd_break_into_debugger
//============================================================

void osd_break_into_debugger(const char *message)
{
	printf("Ignoring MAME exception: %s\n", message);
}

//============================================================
//  PROTOTYPES
//============================================================


static osd_ticks_t init_cycle_counter(void);
static osd_ticks_t performance_cycle_counter(void);

//============================================================
//  STATIC VARIABLES
//============================================================

// global cycle_counter function and divider
static osd_ticks_t      (*cycle_counter)(void) = init_cycle_counter;
static osd_ticks_t      (*ticks_counter)(void) = init_cycle_counter;
static osd_ticks_t      ticks_per_second;

//============================================================
//  init_cycle_counter
//
//  to avoid total grossness, this function is split by subarch
//============================================================

static osd_ticks_t init_cycle_counter(void)
{
	osd_ticks_t start, end;
	osd_ticks_t a, b;

	ULONG  frequency;
	PTIB   ptib;
	ULONG  ulClass;
	ULONG  ulDelta;

	DosGetInfoBlocks( &ptib, NULL );
	ulClass = HIBYTE( ptib->tib_ptib2->tib2_ulpri );
	ulDelta = LOBYTE( ptib->tib_ptib2->tib2_ulpri );

	if ( DosTmrQueryFreq( &frequency ) == 0 )
	{
		// use performance counter if available as it is constant
		cycle_counter = performance_cycle_counter;
		ticks_counter = performance_cycle_counter;

		ticks_per_second = frequency;

		// return the current cycle count
		return (*cycle_counter)();
	}
	else
	{
		fprintf(stderr, "No Timer available!\n");
		exit(-1);
	}

	// temporarily set our priority higher
	DosSetPriority( PRTYS_THREAD, PRTYC_TIMECRITICAL, PRTYD_MAXIMUM, 0 );

	// wait for an edge on the timeGetTime call
	a = SDL_GetTicks();
	do
	{
		b = SDL_GetTicks();
	} while (a == b);

	// get the starting cycle count
	start = (*cycle_counter)();

	// now wait for 1/4 second total
	do
	{
		a = SDL_GetTicks();
	} while (a - b < 250);

	// get the ending cycle count
	end = (*cycle_counter)();

	// compute ticks_per_sec
	ticks_per_second = (end - start) * 4;

	// restore our priority
	DosSetPriority( PRTYS_THREAD, ulClass, ulDelta, 0 );

	// return the current cycle count
	return (*cycle_counter)();
}

//============================================================
//  performance_cycle_counter
//============================================================

static osd_ticks_t performance_cycle_counter(void)
{
	QWORD qwTime;

	DosTmrQueryTime( &qwTime );
	return (osd_ticks_t)qwTime.ulLo;
}

//============================================================
//   osd_ticks
//============================================================

osd_ticks_t osd_ticks(void)
{
	return (*cycle_counter)();
}


//============================================================
//  osd_ticks_per_second
//============================================================

osd_ticks_t osd_ticks_per_second(void)
{
	if (ticks_per_second == 0)
	{
		// if we haven't computed the value yet, there's no time like the present
		init_cycle_counter();
	}
	return ticks_per_second;
}


//============================================================
//  osd_sleep
//============================================================

void osd_sleep(osd_ticks_t duration)
{
	UINT32 msec;

	// make sure we've computed ticks_per_second
	if (ticks_per_second == 0)
		(void)osd_ticks();

	// convert to milliseconds, rounding down
	msec = (UINT32)(duration * 1000 / ticks_per_second);

	// only sleep if at least 2 full milliseconds
	if (msec >= 2)
	{
		// take a couple of msecs off the top for good measure
		msec -= 2;
		usleep(msec*1000);
	}
}
