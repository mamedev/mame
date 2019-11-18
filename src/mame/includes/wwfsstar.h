// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_INCLUDES_WWFSSTAR_H
#define MAME_INCLUDES_WWFSSTAR_H

#pragma once

#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class wwfsstar_state : public driver_device
{
public:
	wwfsstar_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_spriteram(*this, "spriteram"),
		m_fg0_videoram(*this, "fg0_videoram"),
		m_bg0_videoram(*this, "bg0_videoram")
	{ }

	void wwfsstar(machine_config &config);

	DECLARE_READ_LINE_MEMBER(vblank_r);

protected:
	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_fg0_videoram;
	required_shared_ptr<uint16_t> m_bg0_videoram;

	int m_vblank;
	int m_scrollx;
	int m_scrolly;
	tilemap_t *m_fg0_tilemap;
	tilemap_t *m_bg0_tilemap;

	DECLARE_WRITE16_MEMBER(scroll_w);
	DECLARE_WRITE16_MEMBER(flipscreen_w);
	DECLARE_WRITE16_MEMBER(irqack_w);
	DECLARE_WRITE16_MEMBER(fg0_videoram_w);
	DECLARE_WRITE16_MEMBER(bg0_videoram_w);

	TIMER_DEVICE_CALLBACK_MEMBER(scanline);

	TILE_GET_INFO_MEMBER(get_fg0_tile_info);
	TILEMAP_MAPPER_MEMBER(bg0_scan);
	TILE_GET_INFO_MEMBER(get_bg0_tile_info);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );

	void main_map(address_map &map);
	void sound_map(address_map &map);
};

#endif // MAME_INCLUDES_WWFSSTAR_H
