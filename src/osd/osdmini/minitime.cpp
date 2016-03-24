// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  minitime.c - Minimal core timing functions
//
//============================================================

#include "osdepend.h"
#include <time.h>


//============================================================
//  osd_num_processors
//============================================================

int osd_get_num_processors(void)
{
	return 1;
}

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
//  osd_sleep
//============================================================

void osd_sleep(osd_ticks_t duration)
{
	// if there was a generic, cross-platform way to give up
	// time, this is where we would do it
}
