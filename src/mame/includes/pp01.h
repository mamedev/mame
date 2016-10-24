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
	uint8_t m_video_scroll;
	uint8_t m_memory_block[16];
	uint8_t m_video_write_mode;
	uint8_t m_key_line;
	void pp01_video_write_mode_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pp01_video_r_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pp01_video_g_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pp01_video_b_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pp01_video_r_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pp01_video_g_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pp01_video_b_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pp01_mem_block_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t pp01_mem_block_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_pp01(palette_device &palette);
	uint32_t screen_update_pp01(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void pp01_pit_out0(int state);
	void pp01_pit_out1(int state);
	uint8_t pp01_8255_porta_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pp01_8255_porta_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t pp01_8255_portb_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pp01_8255_portb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pp01_8255_portc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t pp01_8255_portc_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pp01_video_w(uint8_t block,uint16_t offset,uint8_t data,uint8_t part);
	void pp01_set_memory(uint8_t block, uint8_t data);
};

#endif
