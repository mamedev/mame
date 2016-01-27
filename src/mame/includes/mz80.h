// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************
 *
 * includes/mz80.h
 *
 ****************************************************************************/

#ifndef MZ80_H_
#define MZ80_H_

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/pit8253.h"
#include "imagedev/cassette.h"
#include "sound/speaker.h"
#include "sound/wave.h"

class mz80_state : public driver_device
{
public:
	mz80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pit(*this, "pit8253"),
		m_ppi(*this, "ppi8255"),
		m_cassette(*this, "cassette"),
		m_speaker(*this, "speaker"),
		m_p_ram(*this, "p_ram"),
		m_p_videoram(*this, "p_videoram"){ }

	required_device<cpu_device> m_maincpu;
	required_device<pit8253_device> m_pit;
	required_device<i8255_device> m_ppi;
	required_device<cassette_image_device> m_cassette;
	required_device<speaker_sound_device> m_speaker;
	DECLARE_READ8_MEMBER(mz80k_strobe_r);
	DECLARE_WRITE8_MEMBER(mz80k_strobe_w);
	DECLARE_READ8_MEMBER(mz80k_8255_portb_r);
	DECLARE_READ8_MEMBER(mz80k_8255_portc_r);
	DECLARE_WRITE8_MEMBER(mz80k_8255_porta_w);
	DECLARE_WRITE8_MEMBER(mz80k_8255_portc_w);
	DECLARE_WRITE_LINE_MEMBER(pit_out0_changed);
	DECLARE_WRITE_LINE_MEMBER(pit_out2_changed);
	bool m_mz80k_vertical;
	bool m_mz80k_tempo_strobe;
	UINT8 m_speaker_level;
	bool m_prev_state;
	UINT8 m_mz80k_cursor_cnt;
	UINT8 m_mz80k_keyboard_line;
	required_shared_ptr<UINT8> m_p_ram;
	const UINT8 *m_p_chargen;
	required_shared_ptr<UINT8> m_p_videoram;
	DECLARE_DRIVER_INIT(mz80k);
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_mz80k(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_mz80kj(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_mz80a(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(ne555_tempo_callback);
};


/*----------- defined in video/mz80.c -----------*/

extern const gfx_layout mz80k_charlayout;
extern const gfx_layout mz80kj_charlayout;

#endif /* MZ80_H_ */
