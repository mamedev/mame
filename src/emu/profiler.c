/***************************************************************************

    profiler.c

    Functions to manage profiling of MAME execution.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

****************************************************************************

    Profiling is scope-based. To start profiling, put a profiler_scope
    object on the stack. To end profiling, just end the scope:

    {
        profiler_scope scope(PROFILER_VIDEO);

        your_work_here();
    }

    the profiler handles a FILO list so calls may be nested.

***************************************************************************/

#include "emu.h"
#include "profiler.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

struct profile_string
{
	int 		type;
	const char *string;
};



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

profiler_state g_profiler;



//**************************************************************************
//  DUMMY PROFILER STATE
//**************************************************************************

//-------------------------------------------------
//  dummy_profiler_state - constructor
//-------------------------------------------------

dummy_profiler_state::dummy_profiler_state()
{
}



//**************************************************************************
//  REAL PROFILER STATE
//**************************************************************************

//-------------------------------------------------
//  real_profiler_state - constructor
//-------------------------------------------------

real_profiler_state::real_profiler_state()
	: m_enabled(false),
	  m_dataready(false),
	  m_filoindex(0),
	  m_dataindex(0)
{
	memset(m_filo, 0, sizeof(m_filo));
	memset(m_data, 0, sizeof(m_data));
}


//-------------------------------------------------
//  real_start - mark the beginning of a
//  profiler entry
//-------------------------------------------------

void real_profiler_state::real_start(profile_type type)
{
	osd_ticks_t curticks = get_profile_ticks();

	// track context switches
	history_data &data = m_data[m_dataindex];
	if (type >= PROFILER_DEVICE_FIRST && type <= PROFILER_DEVICE_MAX)
		data.context_switches++;

	// we're starting a new bucket, begin now
	int index = m_filoindex++;
	filo_entry &entry = m_filo[index];

	// fail if we overflow
	if (index > ARRAY_LENGTH(m_filo))
		throw emu_fatalerror("Profiler FILO overflow (type = %d)\n", type);

	// if we're nested, stop the previous entry
	if (index > 0)
	{
		filo_entry &preventry = m_filo[index - 1];
		data.duration[preventry.type] += curticks - preventry.start;
	}

	// fill in this entry
	entry.type = type;
	entry.start = curticks;
}


//-------------------------------------------------
//  real_stop - mark the end of a profiler entry
//-------------------------------------------------

void real_profiler_state::real_stop()
{
	osd_ticks_t curticks = get_profile_ticks();

	// we're ending an existing bucket, update the time
	if (m_filoindex > 0)
	{
		int index = --m_filoindex;
		filo_entry &entry = m_filo[index];

		// account for the time taken
		history_data &data = m_data[m_dataindex];
		data.duration[entry.type] += curticks - entry.start;

		// if we have a previous entry, restart his time now
		if (index != 0)
		{
			filo_entry &preventry = m_filo[index - 1];
			preventry.start = curticks;
		}
	}
}


//-------------------------------------------------
//  text - return the current text in an astring
//-------------------------------------------------

const char *real_profiler_state::text(running_machine &machine, astring &string)
{
	static const profile_string names[] =
	{
		{ PROFILER_DRC_COMPILE,      "DRC Compilation" },
		{ PROFILER_MEM_REMAP,        "Memory Remapping" },
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

	g_profiler.start(PROFILER_PROFILER);

	// compute the total time for all bits, not including profiler or idle
	UINT64 computed = 0;
	profile_type curtype;
	for (curtype = PROFILER_DEVICE_FIRST; curtype < PROFILER_PROFILER; curtype++)
		for (int curmem = 0; curmem < ARRAY_LENGTH(m_data); curmem++)
			computed += m_data[curmem].duration[curtype];

	// save that result in normalize, and continue adding the rest
	UINT64 normalize = computed;
	for ( ; curtype < PROFILER_TOTAL; curtype++)
		for (int curmem = 0; curmem < ARRAY_LENGTH(m_data); curmem++)
			computed += m_data[curmem].duration[curtype];

	// this becomes the total; if we end up with 0 for anything, we were just started, so return empty
	UINT64 total = computed;
	string.reset();
	if (total == 0 || normalize == 0)
	{
		g_profiler.stop();
		return string;
	}

	// loop over all types and generate the string
	for (curtype = PROFILER_DEVICE_FIRST; curtype < PROFILER_TOTAL; curtype++)
	{
		// determine the accumulated time for this type
		computed = 0;
		for (int curmem = 0; curmem < ARRAY_LENGTH(m_data); curmem++)
			computed += m_data[curmem].duration[curtype];

		// if we have non-zero data and we're ready to display, do it
		if (m_dataready && computed != 0)
		{
			// start with the un-normalized percentage
			string.catprintf("%02d%% ", (int)((computed * 100 + total/2) / total));

			// followed by the normalized percentage for everything but profiler and idle
			if (curtype < PROFILER_PROFILER)
				string.catprintf("%02d%% ", (int)((computed * 100 + normalize/2) / normalize));

			// and then the text
			if (curtype >= PROFILER_DEVICE_FIRST && curtype <= PROFILER_DEVICE_MAX)
				string.catprintf("'%s'", machine.m_devicelist.find(curtype - PROFILER_DEVICE_FIRST)->tag());
			else
				for (int nameindex = 0; nameindex < ARRAY_LENGTH(names); nameindex++)
					if (names[nameindex].type == curtype)
					{
						string.cat(names[nameindex].string);
						break;
					}

			// followed by a carriage return
			string.cat("\n");
		}
	}

	// followed by context switches
	if (m_dataready)
	{
		int switches = 0;
		for (int curmem = 0; curmem < ARRAY_LENGTH(m_data); curmem++)
			switches += m_data[curmem].context_switches;
		string.catprintf("%d CPU switches\n", switches / (int) ARRAY_LENGTH(m_data));
	}

	// advance to the next dataset and reset it to 0
	m_dataindex = (m_dataindex + 1) % ARRAY_LENGTH(m_data);
	memset(&m_data[m_dataindex], 0, sizeof(m_data[m_dataindex]));

	// we are ready once we have wrapped around
	if (m_dataindex == 0)
		m_dataready = true;

	g_profiler.stop();
	return string;
}
