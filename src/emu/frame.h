// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    video.h

    Core MAME frame handling routines.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef MAME_EMU_FRAME_H
#define MAME_EMU_FRAME_H


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// number of levels of frameskipping supported
constexpr int FRAMESKIP_LEVELS = 12;
constexpr int MAX_FRAMESKIP = FRAMESKIP_LEVELS - 2;


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class running_machine;

// ======================> frame_manager

class frame_manager
{
public:
	// construction/destruction
	frame_manager(running_machine &machine);

	// getters
	running_machine &machine() const { return m_machine; }
	bool skip_this_frame() const { return m_skipping_this_frame; }
	int frameskip() const { return m_auto_frameskip ? -1 : m_frameskip_level; }
	osd_ticks_t average_oversleep() const { return m_average_oversleep; }
	bool auto_frameskip() const { return m_auto_frameskip; }
	u8 frameskip_level() const { return m_frameskip_level; }

	// setters
	void set_frameskip(int frameskip);
	void set_average_oversleep(osd_ticks_t value) { m_average_oversleep = value; }

	// speed and throttling helpers
	void update_frameskip();

	// single stepping
	void step_single_frame();
	bool is_single_stepping() const { return m_single_step; }

	bool frame_update();

private:
	// internal state
	running_machine &	m_machine;                  // reference to our machine

	// frameskipping
	u8                  m_empty_skip_count;         // number of empty frames we have skipped
	u8                  m_frameskip_level;          // current frameskip level
	u8                  m_frameskip_counter;        // counter that counts through the frameskip steps
	s8                  m_frameskip_adjust;
	bool                m_skipping_this_frame;      // flag: true if we are skipping the current frame
	osd_ticks_t         m_average_oversleep;        // average number of ticks the OSD oversleeps

	// configuration
	bool                m_auto_frameskip;           // flag: true if we're automatically frameskipping

	// single stepping
	bool                m_single_step;				// are we currently single stepping?

	static const bool   s_skiptable[FRAMESKIP_LEVELS][FRAMESKIP_LEVELS];
};

#endif // MAME_EMU_FRAME_H
