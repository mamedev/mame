// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/***************************************************************************

    Double Dribble

***************************************************************************/
#ifndef MAME_INCLUDES_DDRIBBLE_H
#define MAME_INCLUDES_DDRIBBLE_H

#pragma once

#include "sound/flt_rc.h"
#include "sound/vlm5030.h"
#include "emupal.h"
#include "tilemap.h"

class ddribble_state : public driver_device
{
public:
	ddribble_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_fg_videoram(*this, "fg_videoram"),
		m_spriteram_1(*this, "spriteram_1"),
		m_sharedram(*this, "sharedram"),
		m_bg_videoram(*this, "bg_videoram"),
		m_spriteram_2(*this, "spriteram_2"),
		m_snd_sharedram(*this, "snd_sharedram"),
		m_maincpu(*this, "maincpu"),
		m_cpu1(*this, "cpu1"),
		m_vlm(*this, "vlm"),
		m_filter1(*this, "filter1"),
		m_filter2(*this, "filter2"),
		m_filter3(*this, "filter3"),
		m_gfxdecode(*this, "gfxdecode")
	{ }

	void ddribble(machine_config &config);

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_fg_videoram;
	required_shared_ptr<uint8_t> m_spriteram_1;
	required_shared_ptr<uint8_t> m_sharedram;
	required_shared_ptr<uint8_t> m_bg_videoram;
	required_shared_ptr<uint8_t> m_spriteram_2;
	required_shared_ptr<uint8_t> m_snd_sharedram;

	/* video-related */
	tilemap_t     *m_fg_tilemap;
	tilemap_t     *m_bg_tilemap;
	int         m_vregs[2][5];
	int         m_charbank[2];

	/* misc */
	int         m_int_enable_0;
	int         m_int_enable_1;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_cpu1;
	required_device<vlm5030_device> m_vlm;
	required_device<filter_rc_device> m_filter1;
	required_device<filter_rc_device> m_filter2;
	required_device<filter_rc_device> m_filter3;
	required_device<gfxdecode_device> m_gfxdecode;

	DECLARE_WRITE8_MEMBER(ddribble_bankswitch_w);
	DECLARE_READ8_MEMBER(ddribble_sharedram_r);
	DECLARE_WRITE8_MEMBER(ddribble_sharedram_w);
	DECLARE_READ8_MEMBER(ddribble_snd_sharedram_r);
	DECLARE_WRITE8_MEMBER(ddribble_snd_sharedram_w);
	DECLARE_WRITE8_MEMBER(ddribble_coin_counter_w);
	DECLARE_WRITE8_MEMBER(K005885_0_w);
	DECLARE_WRITE8_MEMBER(K005885_1_w);
	DECLARE_WRITE8_MEMBER(ddribble_fg_videoram_w);
	DECLARE_WRITE8_MEMBER(ddribble_bg_videoram_w);
	DECLARE_READ8_MEMBER(ddribble_vlm5030_busy_r);
	DECLARE_WRITE8_MEMBER(ddribble_vlm5030_ctrl_w);
	TILEMAP_MAPPER_MEMBER(tilemap_scan);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void ddribble_palette(palette_device &palette) const;
	uint32_t screen_update_ddribble(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(vblank_irq);
	void draw_sprites(  bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t* source, int lenght, int gfxset, int flipscreen );
	void cpu0_map(address_map &map);
	void cpu1_map(address_map &map);
	void cpu2_map(address_map &map);
	void vlm_map(address_map &map);
};

#endif // MAME_INCLUDES_DDRIBBLE_H
