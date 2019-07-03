// license:BSD-3-Clause
// copyright-holders:hap
/******************************************************************************
*
*  Fidelity Electronics chess computers base driver
*  implementation is in machine/fidelbase.cpp
*
******************************************************************************/

#ifndef MAME_INCLUDES_FIDELBASE_H
#define MAME_INCLUDES_FIDELBASE_H

#pragma once

#include "includes/chessbase.h"

#include "machine/bankdev.h"

class fidelbase_state : public chessbase_state
{
public:
	fidelbase_state(const machine_config &mconfig, device_type type, const char *tag) :
		chessbase_state(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mainmap(*this, "mainmap")
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(div_changed) { div_refresh(newval); }

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	// devices/pointers
	required_device<cpu_device> m_maincpu;
	optional_device<address_map_bank_device> m_mainmap;

	// dynamic cpu divider
	void div_refresh(ioport_value val = 0xff);
	void div_trampoline_w(offs_t offset, u8 data);
	u8 div_trampoline_r(offs_t offset);

	void div_trampoline(address_map &map);

private:
	inline void div_set_cpu_freq(offs_t offset);

	u16 m_div_status;
	ioport_value m_div_config;
	double m_div_scale;
	emu_timer *m_div_timer;
};


INPUT_PORTS_EXTERN( fidel_cpu_div_2 );
INPUT_PORTS_EXTERN( fidel_cpu_div_4 );

#endif // MAME_INCLUDES_FIDELBASE_H
