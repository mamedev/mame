// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, David Haywood

/*************************************************************************

    Double Dragon 3 & The Combatribes

*************************************************************************/
#ifndef MAME_TECHNOS_DDRAGON3_H
#define MAME_TECHNOS_DDRAGON3_H

#pragma once

#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "sound/okim6295.h"
#include "video/bufsprite.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"


class ddragon3_state : public driver_device
{
public:
	ddragon3_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_bg_videoram(*this, "bg_videoram"),
		m_fg_videoram(*this, "fg_videoram"),
		m_spriteram(*this, "spriteram"),
		m_oki(*this, "oki"),
		m_screen(*this, "screen")
	{
		vblank_level = 6;
		raster_level = 5;
	}

	void ctribe(machine_config &config);
	void ddragon3b(machine_config &config);
	void ddragon3(machine_config &config);

	TIMER_DEVICE_CALLBACK_MEMBER(ddragon3_scanline);

protected:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	/* misc */
	uint8_t m_pri = 0U;
	int vblank_level;
	int raster_level;

	/* video-related */
	tilemap_t         *m_fg_tilemap = nullptr;
	tilemap_t         *m_bg_tilemap = nullptr;
	uint16_t          m_vreg = 0U;
	uint16_t          m_bg_scrollx = 0U;
	uint16_t          m_bg_scrolly = 0U;
	uint16_t          m_fg_scrollx = 0U;
	uint16_t          m_fg_scrolly = 0U;
	uint16_t          m_bg_tilebase = 0U;

	uint16_t m_bg0_dx = 0U;
	uint16_t m_bg1_dx[2]{};
	uint16_t m_sprite_xoff = 0U;

	/* memory pointers */
	required_shared_ptr<uint16_t> m_bg_videoram;
	required_shared_ptr<uint16_t> m_fg_videoram;
//  required_shared_ptr<uint16_t> m_spriteram;
	required_device<buffered_spriteram16_device> m_spriteram;

	/* devices */
	required_device<okim6295_device> m_oki;
	required_device<screen_device> m_screen;

	void ddragon3_scroll_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t ddragon3_scroll_r(offs_t offset);
	void ddragon3_bg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void ddragon3_fg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void sound_map(address_map &map) ATTR_COLD;

	virtual void video_start() override ATTR_COLD;

private:
	void ddragon3_vreg_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void irq6_ack_w(uint16_t data);
	void irq5_ack_w(uint16_t data);
	void oki_bankswitch_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	uint32_t screen_update_ddragon3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_ctribe(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void ctribe_map(address_map &map) ATTR_COLD;
	void ctribe_sound_map(address_map &map) ATTR_COLD;
	void dd3b_map(address_map &map) ATTR_COLD;
	void ddragon3_map(address_map &map) ATTR_COLD;
};


class wwfwfest_state : public ddragon3_state
{
public:
	wwfwfest_state(const machine_config &mconfig, device_type type, const char *tag) :
		ddragon3_state(mconfig, type, tag),
		m_fg0_videoram(*this, "fg0_videoram"),
		m_paletteram(*this, "palette"),
		m_dsw(*this, "DSW%u", 1U)
	{
		vblank_level = 3;
		raster_level = 2;
	}

	void wwfwfest(machine_config &config);
	void wwfwfstb(machine_config &config);

	template <int N> ioport_value dsw_3f_r();
	template <int N> ioport_value dsw_c0_r();

private:
	/* wwfwfest has an extra layer */
	required_shared_ptr<uint16_t> m_fg0_videoram;
	required_shared_ptr<uint16_t> m_paletteram;
	required_ioport_array<2> m_dsw;
	tilemap_t *m_fg0_tilemap = nullptr;
	void wwfwfest_fg0_videoram_w(offs_t offset, uint16_t data);

	//required_device<buffered_spriteram16_device> m_spriteram;
	void wwfwfest_priority_w(uint8_t data);
	void wwfwfest_irq_ack_w(offs_t offset, uint16_t data);
	void wwfwfest_flipscreen_w(uint16_t data);
	uint16_t wwfwfest_paletteram_r(offs_t offset);
	void wwfwfest_paletteram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void wwfwfest_soundwrite(uint16_t data);

	TILE_GET_INFO_MEMBER(get_fg0_tile_info);

	virtual void video_start() override ATTR_COLD;
	DECLARE_VIDEO_START(wwfwfstb);
	uint32_t screen_update_wwfwfest(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
};

#endif // MAME_TECHNOS_DDRAGON3_H
