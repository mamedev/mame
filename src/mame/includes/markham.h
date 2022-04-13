// license:BSD-3-Clause
// copyright-holders:Uki
/*************************************************************************

    Markham (c) 1983 Sun Electronics
    Strength & Skill (c) 1984 Sun Electronics

*************************************************************************/

#ifndef MAME_INCLUDES_MARKHAM_H
#define MAME_INCLUDES_MARKHAM_H

#pragma once

#include "machine/timer.h"

#include "cpu/z80/z80.h"
#include "cpu/mb88xx/mb88xx.h"
#include "sound/sn76496.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

class markham_state : public driver_device
{
public:
	// construction/destruction
	markham_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_subcpu(*this, "subcpu")
		, m_mcu(*this, "mcu")
		, m_sn(*this, "sn%u", 1U)
		, m_screen(*this, "screen")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_spriteram(*this, "spriteram")
		, m_videoram(*this, "videoram")
		, m_xscroll(*this, "xscroll")
		, m_scroll_ctrl(0)
		, m_irq_source(0)
		, m_irq_scanline_start(0)
		, m_irq_scanline_end(0)
		, m_coin2_lock_cnt(3)
		, m_packet_buffer{}
		, m_packet_write_pos(0)
		, m_packet_reset(true)
	{
	}

	void markham(machine_config &config);
	void strnskil(machine_config &config);
	void banbam(machine_config &config);

	void init_common();
	void init_banbam();
	void init_pettanp();

private:
	void base_master_map(address_map &map);
	void markham_master_map(address_map &map);
	void strnskil_master_map(address_map &map);
	void banbam_master_map(address_map &map);
	void markham_slave_map(address_map &map);
	void strnskil_slave_map(address_map &map);

	void coin_output_w(uint8_t data);
	void flipscreen_w(uint8_t data);
	void videoram_w(offs_t offset, uint8_t data);

	// markham specific
	uint8_t markham_e004_r();

	// strnskil specific
	uint8_t strnskil_d800_r();
	void strnskil_master_output_w(uint8_t data);

	// protection comms for banbam/pettanp
	uint8_t banbam_protection_r();
	void banbam_protection_w(uint8_t data);
	void mcu_reset_w(uint8_t data);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	uint32_t screen_update_markham(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_strnskil(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	void markham_palette(palette_device &palette) const;
	DECLARE_VIDEO_START(strnskil);

	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	TIMER_DEVICE_CALLBACK_MEMBER(strnskil_scanline);

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	optional_device<mb8841_cpu_device> m_mcu;
	required_device_array<sn76496_device, 2> m_sn;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_xscroll;

	/* video-related */
	tilemap_t *m_bg_tilemap = nullptr;

	uint8_t m_scroll_ctrl;
	uint8_t m_irq_source;
	uint8_t m_irq_scanline_start;
	uint8_t m_irq_scanline_end;

	/* misc */
	uint8_t m_coin2_lock_cnt;

	/* banbam protection simulation */
	uint8_t m_packet_buffer[2];
	uint8_t m_packet_write_pos;
	bool m_packet_reset;

	u8 m_strnskil_slave_irq = 0;
};

#endif // MAME_INCLUDES_MARKHAM_H
