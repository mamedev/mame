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
#include "machine/timer.h"
#include "sound/dac.h"
#include "sound/s14001a.h"

class fidelbase_state : public chessbase_state
{
public:
	fidelbase_state(const machine_config &mconfig, device_type type, const char *tag) :
		chessbase_state(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mainmap(*this, "mainmap"),
		m_speech(*this, "speech"),
		m_speech_rom(*this, "speech"),
		m_language(*this, "language"),
		m_dac(*this, "dac")
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(div_changed) { div_refresh(newval); }

protected:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	optional_device<address_map_bank_device> m_mainmap;
	optional_device<s14001a_device> m_speech;
	optional_region_ptr<u8> m_speech_rom;
	optional_region_ptr<u8> m_language;
	optional_device<dac_bit_interface> m_dac;

	u8 m_speech_data;
	u8 m_speech_bank; // speech rom higher address bits

	// dynamic cpu divider
	void div_trampoline_w(offs_t offset, u8 data);
	u8 div_trampoline_r(offs_t offset);
	inline void div_set_cpu_freq(offs_t offset);
	void div_trampoline(address_map &map);
	void div_refresh(ioport_value val = 0xff);
	u16 m_div_status;
	ioport_value m_div_config;
	double m_div_scale;
	emu_timer *m_div_timer;

	virtual void machine_start() override;
	virtual void machine_reset() override;
};


INPUT_PORTS_EXTERN( fidel_cpu_div_2 );
INPUT_PORTS_EXTERN( fidel_cpu_div_4 );

#endif // MAME_INCLUDES_FIDELBASE_H
