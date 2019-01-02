// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/*****************************************************************************
 *
 * includes/llc.h
 *
 ****************************************************************************/

#ifndef MAME_INCLUDES_LLC_H
#define MAME_INCLUDES_LLC_H

#pragma once


#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"
#include "machine/k7659kb.h"
#include "machine/ram.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "sound/spkrdev.h"

class llc_state : public driver_device
{
public:
	llc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_speaker(*this, "speaker")
		, m_p_videoram(*this, "videoram")
		, m_maincpu(*this, "maincpu")
		, m_ram(*this, RAM_TAG)
		, m_p_chargen(*this, "chargen")
		, m_digits(*this, "digit%u", 0U)
	{ }

	void llc1(machine_config &config);
	void llc2(machine_config &config);

	void init_llc2();
	void init_llc1();

private:
	DECLARE_WRITE8_MEMBER(llc2_rom_disable_w);
	DECLARE_WRITE8_MEMBER(llc2_basic_enable_w);
	void kbd_put(u8 data);
	DECLARE_READ8_MEMBER(llc1_port1_a_r);
	DECLARE_READ8_MEMBER(llc1_port2_a_r);
	DECLARE_READ8_MEMBER(llc1_port2_b_r);
	DECLARE_WRITE8_MEMBER(llc1_port1_a_w);
	DECLARE_WRITE8_MEMBER(llc1_port1_b_w);
	DECLARE_READ8_MEMBER(llc2_port1_b_r);
	DECLARE_READ8_MEMBER(llc2_port2_a_r);
	DECLARE_WRITE8_MEMBER(llc2_port1_b_w);
	DECLARE_MACHINE_START(llc1);
	DECLARE_MACHINE_RESET(llc1);
	DECLARE_MACHINE_RESET(llc2);
	uint32_t screen_update_llc1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_llc2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void llc1_io(address_map &map);
	void llc1_mem(address_map &map);
	void llc2_io(address_map &map);
	void llc2_mem(address_map &map);

	bool m_rv;
	uint8_t m_term_status;
	uint8_t m_llc1_key;
	uint8_t m_porta;
	uint8_t m_term_data;
	optional_device<speaker_sound_device> m_speaker;
	optional_shared_ptr<uint8_t> m_p_videoram;
	required_device<z80_device> m_maincpu;
	optional_device<ram_device> m_ram;
	required_region_ptr<u8> m_p_chargen;
	output_finder<8> m_digits;
};

#endif // MAME_INCLUDES_LLC_H
