// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************
 *
 * includes/mz80.h
 *
 ****************************************************************************/
#ifndef MAME_SHARP_MZ80_H
#define MAME_SHARP_MZ80_H

#pragma once

#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/pit8253.h"
#include "machine/timer.h"
#include "imagedev/cassette.h"
#include "sound/spkrdev.h"

class mz80_state : public driver_device
{
public:
	mz80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pit(*this, "pit8253")
		, m_ppi(*this, "ppi8255")
		, m_cassette(*this, "cassette")
		, m_speaker(*this, "speaker")
		, m_p_ram(*this, "p_ram")
		, m_p_videoram(*this, "videoram")
		, m_p_chargen(*this, "chargen")
		, m_keyboard(*this, "LINE%d", 0U)
	{ }

	void mz80kj(machine_config &config);
	void mz80k(machine_config &config);
	void mz80a(machine_config &config);

	void init_mz80k();

private:
	uint8_t mz80k_strobe_r();
	void mz80k_strobe_w(uint8_t data);
	uint8_t mz80k_8255_portb_r();
	uint8_t mz80k_8255_portc_r();
	void mz80k_8255_porta_w(uint8_t data);
	void mz80k_8255_portc_w(uint8_t data);
	void pit_out0_changed(int state);
	void pit_out2_changed(int state);
	uint32_t screen_update_mz80k(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_mz80kj(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_mz80a(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(ne555_tempo_callback);

	void mz80k_io(address_map &map) ATTR_COLD;
	void mz80k_mem(address_map &map) ATTR_COLD;
	bool m_mz80k_vertical = false;
	bool m_mz80k_tempo_strobe = false;
	uint8_t m_speaker_level = 0;
	bool m_prev_state = false;
	uint8_t m_mz80k_cursor_cnt = 0;
	uint8_t m_mz80k_keyboard_line = 0;
	virtual void machine_reset() override ATTR_COLD;
	required_device<cpu_device> m_maincpu;
	required_device<pit8253_device> m_pit;
	required_device<i8255_device> m_ppi;
	required_device<cassette_image_device> m_cassette;
	required_device<speaker_sound_device> m_speaker;
	required_shared_ptr<uint8_t> m_p_ram;
	required_shared_ptr<uint8_t> m_p_videoram;
	required_region_ptr<u8> m_p_chargen;
	required_ioport_array<10> m_keyboard;
};


/*----------- defined in video/mz80.c -----------*/

extern const gfx_layout mz80k_charlayout;
extern const gfx_layout mz80kj_charlayout;

#endif // MAME_SHARP_MZ80_H
