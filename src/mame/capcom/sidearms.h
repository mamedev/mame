// license:BSD-3-Clause
// copyright-holders:Paul Leaman, Curt Coder
#ifndef MAME_CAPCOM_SIDEARMS_H
#define MAME_CAPCOM_SIDEARMS_H

#pragma once

#include "video/bufsprite.h"
#include "emupal.h"
#include "tilemap.h"

class sidearms_state : public driver_device
{
public:
	sidearms_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spriteram(*this, "spriteram") ,
		m_bg_scrollx(*this, "bg_scrollx"),
		m_bg_scrolly(*this, "bg_scrolly"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_ports(*this, { { "SYSTEM", "P1", "P2", "DSW0", "DSW1" } })
	{
	}

	void sidearms(machine_config &config);
	void turtship(machine_config &config);
	void whizz(machine_config &config);

	void init_dyger();
	void init_sidearms();
	void init_whizz();
	void init_turtship();

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<buffered_spriteram8_device> m_spriteram;

	required_shared_ptr<uint8_t> m_bg_scrollx;
	required_shared_ptr<uint8_t> m_bg_scrolly;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;

	optional_ioport_array<5> m_ports;

	int m_gameid = 0;

	uint8_t *m_tilerom = nullptr;
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;

	int m_bgon = 0;
	int m_objon = 0;
	int m_staron = 0;
	int m_charon = 0;
	int m_flipon = 0;

	uint32_t m_hflop_74a_n = 0;
	uint32_t m_hcount_191 = 0;
	uint32_t m_vcount_191 = 0;
	uint32_t m_latch_374 = 0;

	void bankswitch_w(uint8_t data);
	void videoram_w(offs_t offset, uint8_t data);
	void colorram_w(offs_t offset, uint8_t data);
	void c804_w(uint8_t data);
	void gfxctrl_w(uint8_t data);
	void star_scrollx_w(uint8_t data);
	void star_scrolly_w(uint8_t data);

	uint8_t turtship_ports_r(offs_t offset);

	void whizz_bankswitch_w(uint8_t data);

	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	TILE_GET_INFO_MEMBER(get_sidearms_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_philko_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILEMAP_MAPPER_MEMBER(tilemap_scan);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites_region(bitmap_ind16 &bitmap, const rectangle &cliprect, int start_offset, int end_offset );
	void draw_starfield( bitmap_ind16 &bitmap );
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void sidearms_map(address_map &map) ATTR_COLD;
	void sidearms_sound_map(address_map &map) ATTR_COLD;
	void turtship_map(address_map &map) ATTR_COLD;
	void whizz_io_map(address_map &map) ATTR_COLD;
	void whizz_map(address_map &map) ATTR_COLD;
	void whizz_sound_map(address_map &map) ATTR_COLD;
};

#endif // MAME_CAPCOM_SIDEARMS_H
