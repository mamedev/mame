//============================================================
//
//  minitime.c - Minimal core timing functions
//
//  Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//============================================================

#include "osdepend.h"
#include <time.h>



//============================================================
//  osd_ticks
//============================================================

osd_ticks_t osd_ticks(void)
{
	// use the standard library clock function
	return clock();
}


//============================================================
//  osd_ticks_per_second
//============================================================

osd_ticks_t osd_ticks_per_second(void)
{
	return CLOCKS_PER_SEC;
}


//============================================================
//  osd_profiling_ticks
//============================================================

osd_ticks_t osd_profiling_ticks(void)
{
	// on x86 platforms, we should return the value of RDTSC here
	// generically, we fall back to clock(), which hopefully is
	// fast
	return clock();
}


//============================================================
//  osd_sleep
//============================================================

void osd_sleep(osd_ticks_t duration)
{
	// if there was a generic, cross-platform way to give up
	// time, this is where we would do it
}
