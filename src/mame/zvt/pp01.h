// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************
 *
 * includes/pp01.h
 *
 ****************************************************************************/

#ifndef MAME_ZVT_PP01_H
#define MAME_ZVT_PP01_H

#pragma once

#include "cpu/i8085/i8085.h"
#include "machine/ram.h"
#include "machine/i8251.h"
#include "machine/pit8253.h"
#include "machine/i8255.h"
#include "sound/spkrdev.h"
#include "imagedev/cassette.h"
#include "machine/timer.h"
#include "emupal.h"

class pp01_state : public driver_device
{
public:
	pp01_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pit(*this, "pit")
		, m_ppi(*this, "ppi%u", 1U)
		, m_speaker(*this, "speaker")
		, m_ram(*this, RAM_TAG)
		, m_uart(*this, "uart")
		, m_cass(*this, "cassette")
		, m_mainrom(*this, "maincpu")
		, m_bank(*this, "bank%d", 0U)
		, m_io_keyboard(*this, "LINE%d", 0U)
	{ }

	void pp01(machine_config &config);

protected:
	virtual void device_post_load() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void video_write_mode_w(u8 data);
	void video_r_1_w(offs_t offset, u8 data);
	void video_g_1_w(offs_t offset, u8 data);
	void video_b_1_w(offs_t offset, u8 data);
	void video_r_2_w(offs_t offset, u8 data);
	void video_g_2_w(offs_t offset, u8 data);
	void video_b_2_w(offs_t offset, u8 data);
	void mem_block_w(offs_t offset, u8 data);
	u8 mem_block_r(offs_t offset);
	TIMER_DEVICE_CALLBACK_MEMBER(kansas_r);
	void pp01_palette(palette_device &palette) const;
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void z2_w(int state);
	u8 ppi1_porta_r();
	void ppi1_porta_w(u8 data);
	u8 ppi1_portb_r();
	void ppi1_portb_w(u8 data);
	void ppi1_portc_w(u8 data);
	u8 ppi1_portc_r();
	void video_w(u8 block, uint16_t offset, u8 data, bool part);
	void set_memory(u8 block, u8 data);
	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	u8 m_video_scroll = 0;
	u8 m_memory_block[16]{};
	u8 m_video_write_mode = 0;
	u8 m_key_line = 0;
	bool m_txe = false, m_txd = false, m_rts = false, m_casspol = false;
	u8 m_cass_data[4]{};

	required_device<cpu_device> m_maincpu;
	required_device<pit8253_device> m_pit;
	required_device_array<i8255_device, 2> m_ppi;
	required_device<speaker_sound_device> m_speaker;
	required_device<ram_device> m_ram;
	required_device<i8251_device> m_uart;
	required_device<cassette_image_device> m_cass;

	required_region_ptr<u8> m_mainrom;
	required_memory_bank_array<16> m_bank;

	required_ioport_array<17> m_io_keyboard;
};

#endif // MAME_ZVT_PP01_H
