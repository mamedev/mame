// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    frame.cpp

	Core MAME frame handling routines.

***************************************************************************/

#include "emu.h"
#include "machine.h"
#include "emuopts.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// frameskipping tables
const bool frame_manager::s_skiptable[FRAMESKIP_LEVELS][FRAMESKIP_LEVELS] =
{
	{ false, false, false, false, false, false, false, false, false, false, false, false },
	{ false, false, false, false, false, false, false, false, false, false, false, true  },
	{ false, false, false, false, false, true , false, false, false, false, false, true  },
	{ false, false, false, true , false, false, false, true , false, false, false, true  },
	{ false, false, true , false, false, true , false, false, true , false, false, true  },
	{ false, true , false, false, true , false, true , false, false, true , false, true  },
	{ false, true , false, true , false, true , false, true , false, true , false, true  },
	{ false, true , false, true , true , false, true , false, true , true , false, true  },
	{ false, true , true , false, true , true , false, true , true , false, true , true  },
	{ false, true , true , true , false, true , true , true , false, true , true , true  },
	{ false, true , true , true , true , true , false, true , true , true , true , true  },
	{ false, true , true , true , true , true , true , true , true , true , true , true  }
};


//**************************************************************************
//  FRAME MANAGER
//**************************************************************************

//-------------------------------------------------
//  frame_manager - constructor
//-------------------------------------------------

frame_manager::frame_manager(running_machine &machine)
	: m_machine(machine)
	, m_empty_skip_count(0)
	, m_frameskip_level(machine.options().frameskip())
	, m_frameskip_counter(0)
	, m_frameskip_adjust(0)
	, m_skipping_this_frame(false)
	, m_average_oversleep(0)
	, m_single_step(false)
{
}


//-------------------------------------------------
//  update_frameskip - update frameskipping
//  counters and periodically update autoframeskip
//-------------------------------------------------

void frame_manager::update_frameskip()
{
	// if we're throttling and autoframeskip is on, adjust
	if (machine().video().effective_throttle() && machine().video().effective_autoframeskip() && m_frameskip_counter == 0)
	{
		// calibrate the "adjusted speed" based on the target
		double adjusted_speed_percent = machine().video().speed_percent() / (double)machine().video().throttle_rate();

		// if we're too fast, attempt to increase the frameskip
		double speed = machine().video().speed_factor() * 0.001;
		if (adjusted_speed_percent >= 0.995 * speed)
		{
			// but only after 3 consecutive frames where we are too fast
			if (++m_frameskip_adjust >= 3)
			{
				m_frameskip_adjust = 0;
				if (m_frameskip_level > 0)
					m_frameskip_level--;
			}
		}

		// if we're too slow, attempt to increase the frameskip
		else
		{
			// if below 80% speed, be more aggressive
			if (adjusted_speed_percent < 0.80 *  speed)
				m_frameskip_adjust -= (0.90 * speed - machine().video().speed_percent()) / 0.05;

			// if we're close, only force it up to frameskip 8
			else if (m_frameskip_level < 8)
				m_frameskip_adjust--;

			// perform the adjustment
			while (m_frameskip_adjust <= -2)
			{
				m_frameskip_adjust += 2;
				if (m_frameskip_level < MAX_FRAMESKIP)
					m_frameskip_level++;
			}
		}
	}

	// increment the frameskip counter and determine if we will skip the next frame
	m_frameskip_counter = (m_frameskip_counter + 1) % FRAMESKIP_LEVELS;
	m_skipping_this_frame = s_skiptable[machine().video().effective_frameskip()][m_frameskip_counter];
}


//-------------------------------------------------
//  set_frameskip - set the current actual
//  frameskip (-1 means autoframeskip)
//-------------------------------------------------

void frame_manager::set_frameskip(int frameskip)
{
	// -1 means autoframeskip
	if (frameskip == -1)
	{
		m_auto_frameskip = true;
		m_frameskip_level = 0;
	}

	// any other level is a direct control
	else if (frameskip >= 0 && frameskip <= MAX_FRAMESKIP)
	{
		m_auto_frameskip = false;
		m_frameskip_level = frameskip;
	}
}


bool frame_manager::frame_update()
{
	// only render sound and video if we're in the running phase
	machine_phase const phase = machine().phase();
	bool skipped_it = m_skipping_this_frame;
	if (phase == machine_phase::RUNNING && (!machine().paused() || machine().options().update_in_pause()))
	{
		bool anything_changed = machine().video().finish_screen_updates();

		// if none of the screens changed and we haven't skipped too many frames in a row,
		// mark this frame as skipped to prevent throttling; this helps for games that
		// don't update their screen at the monitor refresh rate
		if (!anything_changed && !m_auto_frameskip && m_frameskip_level == 0 && m_empty_skip_count++ < 3)
			skipped_it = true;
		else
			m_empty_skip_count = 0;
	}

	// if we're single-stepping, pause now
	if (m_single_step)
	{
		machine().pause();
		m_single_step = false;
	}

	return skipped_it;
}


//-------------------------------------------------
//  step_single_frame
//-------------------------------------------------

void frame_manager::step_single_frame()
{
	machine().rewind_capture();
	m_single_step = true;
	machine().resume();
}
