// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina,Pierpaolo Prazzoli
#ifndef MAME_EOLITH_EOLITH_SPEEDUP_H
#define MAME_EOLITH_EOLITH_SPEEDUP_H

#pragma once

#include "machine/timer.h"

#include "emupal.h"
#include "screen.h"

class eolith_e1_speedup_state_base : public driver_device
{
public:
	eolith_e1_speedup_state_base(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
	{
	}

	int speedup_vblank_r();
	int stealsee_speedup_vblank_r();

protected:
	void speedup_read();
	void init_speedup() ATTR_COLD;

	TIMER_DEVICE_CALLBACK_MEMBER(eolith_speedup);

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

private:
	// speedups - see eolith/eolith_speedup.cpp
	int m_speedup_address = 0;
	int m_speedup_address2 = 0;
	int m_speedup_resume_scanline = 0;
	int m_speedup_vblank = 0;
	int m_speedup_scanline = 0;
};

#endif // MAME_EOLITH_EOLITH_SPEEDUP_H
