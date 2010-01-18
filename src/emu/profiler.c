/***************************************************************************

    profiler.c

    Functions to manage profiling of MAME execution.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "emu.h"
#include "profiler.h"



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _profile_string profile_string;
struct _profile_string
{
	int 		type;
	const char *string;
};



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

profiler_state global_profiler;



/***************************************************************************
    CORE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    _profiler_mark_start - mark the beginning of a
    profiler entry
-------------------------------------------------*/

void _profiler_mark_start(int type)
{
	profiler_data *data = &global_profiler.data[global_profiler.dataindex];
	osd_ticks_t curticks = get_profile_ticks();
	profiler_filo_entry *entry;
	int index;

	/* track context switches */
	if (type >= PROFILER_CPU_FIRST && type <= PROFILER_CPU_MAX)
		data->context_switches++;

	/* we're starting a new bucket, begin now */
	index = global_profiler.filoindex++;
	entry = &global_profiler.filo[index];

	/* fail if we overflow */
	if (index > ARRAY_LENGTH(global_profiler.filo))
		fatalerror("Profiler FILO overflow (type = %d)\n", type);

	/* if we're nested, stop the previous entry */
	if (index > 0)
	{
		profiler_filo_entry *preventry = entry - 1;
		data->duration[preventry->type] += curticks - preventry->start;
	}

	/* fill in this entry */
	entry->type = type;
	entry->start = curticks;
}


/*-------------------------------------------------
    _profiler_mark_end - mark the end of a
    profiler entry
-------------------------------------------------*/

void _profiler_mark_end(void)
{
	profiler_data *data = &global_profiler.data[global_profiler.dataindex];
	osd_ticks_t curticks = get_profile_ticks();

	/* we're ending an existing bucket, update the time */
	if (global_profiler.filoindex > 0)
	{
		int index = --global_profiler.filoindex;
		profiler_filo_entry *entry = &global_profiler.filo[index];

		/* account for the time taken */
		data->duration[entry->type] += curticks - entry->start;

		/* if we have a previous entry, restart his time now */
		if (index != 0)
		{
			profiler_filo_entry *preventry = entry - 1;
			preventry->start = curticks;
		}
	}
}


/*-------------------------------------------------
    _profiler_get_text - return the current text
    in an astring
-------------------------------------------------*/

astring &_profiler_get_text(running_machine *machine, astring &string)
{
	static const profile_string names[] =
	{
		{ PROFILER_DRC_COMPILE,      "DRC Compilation" },
		{ PROFILER_MEMREAD,          "Memory Read" },
		{ PROFILER_MEMWRITE,         "Memory Write" },
		{ PROFILER_VIDEO,            "Video Update" },
		{ PROFILER_DRAWGFX,          "drawgfx" },
		{ PROFILER_COPYBITMAP,       "copybitmap" },
		{ PROFILER_TILEMAP_DRAW,     "Tilemap Draw" },
		{ PROFILER_TILEMAP_DRAW_ROZ, "Tilemap ROZ Draw" },
		{ PROFILER_TILEMAP_UPDATE,   "Tilemap Update" },
		{ PROFILER_BLIT,             "OSD Blitting" },
		{ PROFILER_SOUND,            "Sound Generation" },
		{ PROFILER_TIMER_CALLBACK,   "Timer Callbacks" },
		{ PROFILER_INPUT,            "Input Processing" },
		{ PROFILER_MOVIE_REC,        "Movie Recording" },
		{ PROFILER_LOGERROR,         "Error Logging" },
		{ PROFILER_EXTRA,            "Unaccounted/Overhead" },
		{ PROFILER_USER1,            "User 1" },
		{ PROFILER_USER2,            "User 2" },
		{ PROFILER_USER3,            "User 3" },
		{ PROFILER_USER4,            "User 4" },
		{ PROFILER_USER5,            "User 5" },
		{ PROFILER_USER6,            "User 6" },
		{ PROFILER_USER7,            "User 7" },
		{ PROFILER_USER8,            "User 8" },
		{ PROFILER_PROFILER,         "Profiler" },
		{ PROFILER_IDLE,             "Idle" }
	};
	UINT64 computed, normalize, total;
	int curtype, curmem, switches;

	profiler_mark_start(PROFILER_PROFILER);

	/* compute the total time for all bits, not including profiler or idle */
	computed = 0;
	for (curtype = 0; curtype < PROFILER_PROFILER; curtype++)
		for (curmem = 0; curmem < ARRAY_LENGTH(global_profiler.data); curmem++)
			computed += global_profiler.data[curmem].duration[curtype];

	/* save that result in normalize, and continue adding the rest */
	normalize = computed;
	for ( ; curtype < PROFILER_TOTAL; curtype++)
		for (curmem = 0; curmem < ARRAY_LENGTH(global_profiler.data); curmem++)
			computed += global_profiler.data[curmem].duration[curtype];

	/* this becomes the total; if we end up with 0 for anything, we were just started, so return empty */
	total = computed;
	string.reset();
	if (total == 0 || normalize == 0)
		goto out;

	/* loop over all types and generate the string */
	for (curtype = 0; curtype < PROFILER_TOTAL; curtype++)
	{
		/* determine the accumulated time for this type */
		computed = 0;
		for (curmem = 0; curmem < ARRAY_LENGTH(global_profiler.data); curmem++)
			computed += global_profiler.data[curmem].duration[curtype];

		/* if we have non-zero data and we're ready to display, do it */
		if (global_profiler.dataready && computed != 0)
		{
			int nameindex;

			/* start with the un-normalized percentage */
			string.catprintf("%02d%% ", (int)((computed * 100 + total/2) / total));

			/* followed by the normalized percentage for everything but profiler and idle */
			if (curtype < PROFILER_PROFILER)
				string.catprintf("%02d%% ", (int)((computed * 100 + normalize/2) / normalize));

			/* and then the text */
			if (curtype >= PROFILER_CPU_FIRST && curtype <= PROFILER_CPU_MAX)
				string.catprintf("CPU '%s'", machine->devicelist.find(CPU, curtype - PROFILER_CPU_FIRST)->tag.cstr());
			else
				for (nameindex = 0; nameindex < ARRAY_LENGTH(names); nameindex++)
					if (names[nameindex].type == curtype)
					{
						string.cat(names[nameindex].string);
						break;
					}

			/* followed by a carriage return */
			string.cat("\n");
		}
	}

	/* followed by context switches */
	if (global_profiler.dataready)
	{
		switches = 0;
		for (curmem = 0; curmem < ARRAY_LENGTH(global_profiler.data); curmem++)
			switches += global_profiler.data[curmem].context_switches;
		string.catprintf("%d CPU switches\n", switches / (int) ARRAY_LENGTH(global_profiler.data));
	}

	/* advance to the next dataset and reset it to 0 */
	global_profiler.dataindex = (global_profiler.dataindex + 1) % ARRAY_LENGTH(global_profiler.data);
	memset(&global_profiler.data[global_profiler.dataindex], 0, sizeof(global_profiler.data[global_profiler.dataindex]));

	/* we are ready once we have wrapped around */
	if (global_profiler.dataindex == 0)
		global_profiler.dataready = TRUE;

out:
	profiler_mark_end();

	return string;
}
