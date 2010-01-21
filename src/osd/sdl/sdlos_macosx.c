//============================================================
//
//  sdlos_*.c - OS specific low level code
//
//  Copyright (c) 1996-2010, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

// standard sdl header
#include <SDL/SDL.h>

#include <unistd.h>

#include <mach/mach.h>
#include <mach/mach_time.h>

// MAME headers
#include "osdcore.h"

//============================================================
//  PROTOTYPES
//============================================================

static osd_ticks_t init_cycle_counter(void);
static osd_ticks_t mach_cycle_counter(void);

//============================================================
//  STATIC VARIABLES
//============================================================

static osd_ticks_t		(*cycle_counter)(void) = init_cycle_counter;
static osd_ticks_t		(*ticks_counter)(void) = init_cycle_counter;
static osd_ticks_t		ticks_per_second;

//============================================================
//  init_cycle_counter
//
//  to avoid total grossness, this function is split by subarch
//============================================================

static osd_ticks_t init_cycle_counter(void)
{
	osd_ticks_t start, end;
	osd_ticks_t a, b;

	cycle_counter = mach_cycle_counter;
	ticks_counter = mach_cycle_counter;

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

	// return the current cycle count
	return (*cycle_counter)();
}

//============================================================
//  performance_cycle_counter
//============================================================

//============================================================
//  mach_cycle_counter
//============================================================
static osd_ticks_t mach_cycle_counter(void)
{
	return mach_absolute_time();
}

//============================================================
//   osd_cycles
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
		return 1;	// this isn't correct, but it prevents the crash
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

//============================================================
//  osd_num_processors
//============================================================

int osd_num_processors(void)
{
	int processors = 1;

	struct host_basic_info host_basic_info;
	unsigned int count;
	kern_return_t r;
	mach_port_t my_mach_host_self;

	count = HOST_BASIC_INFO_COUNT;
	my_mach_host_self = mach_host_self();
	if ( ( r = host_info(my_mach_host_self, HOST_BASIC_INFO, (host_info_t)(&host_basic_info), &count)) == KERN_SUCCESS )
	{
		processors = host_basic_info.avail_cpus;
	}
	mach_port_deallocate(mach_task_self(), my_mach_host_self);

	return processors;
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
//  osd_getenv
//============================================================

char *osd_getenv(const char *name)
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
