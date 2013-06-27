/***************************************************************************

    profiler.h

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

#pragma once

#ifndef __PROFILER_H__
#define __PROFILER_H__

#include "attotime.h"



//**************************************************************************
//  CONSTANTS
//**************************************************************************

enum profile_type
{
	PROFILER_DEVICE_FIRST = 0,
	PROFILER_DEVICE_MAX = PROFILER_DEVICE_FIRST + 256,
	PROFILER_DRC_COMPILE,
	PROFILER_MEM_REMAP,
	PROFILER_MEMREAD,
	PROFILER_MEMWRITE,
	PROFILER_VIDEO,
	PROFILER_DRAWGFX,
	PROFILER_COPYBITMAP,
	PROFILER_TILEMAP_DRAW,
	PROFILER_TILEMAP_DRAW_ROZ,
	PROFILER_TILEMAP_UPDATE,
	PROFILER_BLIT,
	PROFILER_SOUND,
	PROFILER_TIMER_CALLBACK,
	PROFILER_INPUT,             // input.c and inptport.c
	PROFILER_MOVIE_REC,         // movie recording
	PROFILER_LOGERROR,          // logerror
	PROFILER_EXTRA,             // everything else

	// the USER types are available to driver writers to profile
	// custom sections of the code
	PROFILER_USER1,
	PROFILER_USER2,
	PROFILER_USER3,
	PROFILER_USER4,
	PROFILER_USER5,
	PROFILER_USER6,
	PROFILER_USER7,
	PROFILER_USER8,

	PROFILER_PROFILER,
	PROFILER_IDLE,
	PROFILER_TOTAL
};
DECLARE_ENUM_OPERATORS(profile_type);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> real_profiler_state

class real_profiler_state
{
public:
	// construction/destruction
	real_profiler_state();

	// getters
	bool enabled() const { return m_filoptr != NULL; }
	const char *text(running_machine &machine);

	// enable/disable
	void enable(bool state = true)
	{
		if (state != enabled())
		{
			reset(state);
		}
	}

	// start/stop
	void start(profile_type type) { if (enabled()) real_start(type); }
	void stop() { if (enabled()) real_stop(); }

private:
	void reset(bool enabled);
	void update_text(running_machine &machine);

	//-------------------------------------------------
	//  real_start - mark the beginning of a
	//  profiler entry
	//-------------------------------------------------
	ATTR_FORCE_INLINE void real_start(profile_type type)
	{
		// fail if we overflow
		if (m_filoptr >= &m_filo[ARRAY_LENGTH(m_filo) - 1])
			throw emu_fatalerror("Profiler FILO overflow (type = %d)\n", type);

		// get current tick count
		osd_ticks_t curticks = get_profile_ticks();

		// update previous entry
		m_data[m_filoptr->type] += curticks - m_filoptr->start;

		// move to next entry
		m_filoptr++;

		// fill in this entry
		m_filoptr->type = type;
		m_filoptr->start = curticks;
	}

	//-------------------------------------------------
	//  real_stop - mark the end of a profiler entry
	//-------------------------------------------------
	ATTR_FORCE_INLINE void real_stop()
	{
		// degenerate scenario
		if (UNEXPECTED(m_filoptr <= m_filo))
			return;

		// get current tick count
		osd_ticks_t curticks = get_profile_ticks();

		// account for the time taken
		m_data[m_filoptr->type] += curticks - m_filoptr->start;

		// move back an entry
		m_filoptr--;

		// reset previous entry start time
		m_filoptr->start = curticks;
	}

	// an entry in the FILO
	struct filo_entry
	{
		int             type;                       // type of entry
		osd_ticks_t     start;                      // start time
	};

	// internal state
	filo_entry *        m_filoptr;                  // current FILO index
	astring             m_text;                     // profiler text
	attotime            m_text_time;                // profiler text last update
	filo_entry          m_filo[32];                 // array of FILO entries
	osd_ticks_t         m_data[PROFILER_TOTAL + 1]; // array of data
};


// ======================> dummy_profiler_state

class dummy_profiler_state
{
public:
	// construction/destruction
	dummy_profiler_state();

	// getters
	bool enabled() const { return false; }
	const char *text(running_machine &machine) { return ""; }

	// enable/disable
	void enable(bool state = true) { }

	// start/stop
	void start(profile_type type) { }
	void stop() { }
};


// ======================> profiler_state

#ifdef MAME_PROFILER
typedef real_profiler_state profiler_state;
#else
typedef dummy_profiler_state profiler_state;
#endif



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

extern profiler_state g_profiler;


#endif  /* __PROFILER_H__ */
