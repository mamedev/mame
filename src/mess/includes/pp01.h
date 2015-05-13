// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************
 *
 * includes/pp01.h
 *
 ****************************************************************************/

#ifndef PP01_H_
#define PP01_H_

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/ram.h"
#include "machine/i8251.h"
#include "machine/pit8253.h"
#include "machine/i8255.h"
#include "sound/speaker.h"
//#include "sound/wave.h"
//#include "imagedev/cassette.h"

class pp01_state : public driver_device
{
public:
	pp01_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pit(*this, "pit8253"),
		m_speaker(*this, "speaker"),
		m_ram(*this, RAM_TAG) { }

	required_device<cpu_device> m_maincpu;
	required_device<pit8253_device> m_pit;
	required_device<speaker_sound_device> m_speaker;
	required_device<ram_device> m_ram;
	UINT8 m_video_scroll;
	UINT8 m_memory_block[16];
	UINT8 m_video_write_mode;
	UINT8 m_key_line;
	DECLARE_WRITE8_MEMBER(pp01_video_write_mode_w);
	DECLARE_WRITE8_MEMBER(pp01_video_r_1_w);
	DECLARE_WRITE8_MEMBER(pp01_video_g_1_w);
	DECLARE_WRITE8_MEMBER(pp01_video_b_1_w);
	DECLARE_WRITE8_MEMBER(pp01_video_r_2_w);
	DECLARE_WRITE8_MEMBER(pp01_video_g_2_w);
	DECLARE_WRITE8_MEMBER(pp01_video_b_2_w);
	DECLARE_WRITE8_MEMBER(pp01_mem_block_w);
	DECLARE_READ8_MEMBER(pp01_mem_block_r);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	DECLARE_PALETTE_INIT(pp01);
	UINT32 screen_update_pp01(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(pp01_pit_out0);
	DECLARE_WRITE_LINE_MEMBER(pp01_pit_out1);
	DECLARE_READ8_MEMBER(pp01_8255_porta_r);
	DECLARE_WRITE8_MEMBER(pp01_8255_porta_w);
	DECLARE_READ8_MEMBER(pp01_8255_portb_r);
	DECLARE_WRITE8_MEMBER(pp01_8255_portb_w);
	DECLARE_WRITE8_MEMBER(pp01_8255_portc_w);
	DECLARE_READ8_MEMBER(pp01_8255_portc_r);
	void pp01_video_w(UINT8 block,UINT16 offset,UINT8 data,UINT8 part);
	void pp01_set_memory(UINT8 block, UINT8 data);
};

#endif
