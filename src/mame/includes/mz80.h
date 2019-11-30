// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************
 *
 * includes/mz80.h
 *
 ****************************************************************************/
#ifndef MAME_INCLUDES_MZ80_H
#define MAME_INCLUDES_MZ80_H

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
	{ }

	void mz80kj(machine_config &config);
	void mz80k(machine_config &config);
	void mz80a(machine_config &config);

	void init_mz80k();

private:
	DECLARE_READ8_MEMBER(mz80k_strobe_r);
	DECLARE_WRITE8_MEMBER(mz80k_strobe_w);
	DECLARE_READ8_MEMBER(mz80k_8255_portb_r);
	DECLARE_READ8_MEMBER(mz80k_8255_portc_r);
	DECLARE_WRITE8_MEMBER(mz80k_8255_porta_w);
	DECLARE_WRITE8_MEMBER(mz80k_8255_portc_w);
	DECLARE_WRITE_LINE_MEMBER(pit_out0_changed);
	DECLARE_WRITE_LINE_MEMBER(pit_out2_changed);
	uint32_t screen_update_mz80k(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_mz80kj(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_mz80a(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(ne555_tempo_callback);

	void mz80k_io(address_map &map);
	void mz80k_mem(address_map &map);
	bool m_mz80k_vertical;
	bool m_mz80k_tempo_strobe;
	uint8_t m_speaker_level;
	bool m_prev_state;
	uint8_t m_mz80k_cursor_cnt;
	uint8_t m_mz80k_keyboard_line;
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
	required_device<pit8253_device> m_pit;
	required_device<i8255_device> m_ppi;
	required_device<cassette_image_device> m_cassette;
	required_device<speaker_sound_device> m_speaker;
	required_shared_ptr<uint8_t> m_p_ram;
	required_shared_ptr<uint8_t> m_p_videoram;
	required_region_ptr<u8> m_p_chargen;
};


/*----------- defined in video/mz80.c -----------*/

extern const gfx_layout mz80k_charlayout;
extern const gfx_layout mz80kj_charlayout;

#endif // MAME_INCLUDES_MZ80_H
