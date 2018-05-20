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
#include "screen.h"
#include "speaker.h"

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
		, m_coin_unlock(false)
	{
	}

	void init_banbam();
	void init_pettanp();

	void markham(machine_config &config);
	void strnskil(machine_config &config);
	void banbam(machine_config &config);

	void base_master_map(address_map &map);
	void markham_master_map(address_map &map);
	void strnskil_master_map(address_map &map);
	void markham_slave_map(address_map &map);
	void strnskil_slave_map(address_map &map);

protected:
	DECLARE_WRITE8_MEMBER(coin_output_w);
	template<int Bit> DECLARE_WRITE8_MEMBER(flipscreen_w);
	DECLARE_WRITE8_MEMBER(videoram_w);

	// markham specific
	DECLARE_READ8_MEMBER(markham_e004_r);

	// strnskil specific
	DECLARE_READ8_MEMBER(strnskil_d800_r);

	// protection comms for banbam/pettanp
	DECLARE_READ8_MEMBER(pettanp_protection_r);
	DECLARE_READ8_MEMBER(banbam_protection_r);
	DECLARE_WRITE8_MEMBER(protection_w);

	virtual void video_start() override;

	uint32_t screen_update_markham(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_strnskil(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	DECLARE_PALETTE_INIT(markham);
	DECLARE_VIDEO_START(strnskil);

	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	TIMER_DEVICE_CALLBACK_MEMBER(strnskil_scanline);

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	optional_device<cpu_device> m_mcu;
	required_device_array<sn76496_device, 2> m_sn;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_xscroll;

	/* video-related */
	tilemap_t *m_bg_tilemap;

	uint8_t m_scroll_ctrl;
	uint8_t m_irq_source;
	uint8_t m_irq_scanline_start;
	uint8_t m_irq_scanline_end;

	bool m_coin_unlock;
};

#endif // MAME_INCLUDES_MARKHAM_H