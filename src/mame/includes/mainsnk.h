// license:BSD-3-Clause
// copyright-holders:David Haywood, Tomasz Slanina
#ifndef MAME_INCLUDES_MAINSNK_H
#define MAME_INCLUDES_MAINSNK_H

#pragma once

#include "machine/gen_latch.h"
#include "emupal.h"

class mainsnk_state : public driver_device
{
public:
	mainsnk_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_bgram(*this, "bgram"),
		m_spriteram(*this, "spriteram"),
		m_fgram(*this, "fgram")
	{ }

	void mainsnk(machine_config &config);

protected:
	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint8_t> m_bgram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_fgram;

	tilemap_t *m_tx_tilemap;
	tilemap_t *m_bg_tilemap;
	int m_sound_cpu_busy;
	uint32_t m_bg_tile_offset;

	DECLARE_READ8_MEMBER(sound_ack_r);
	DECLARE_WRITE8_MEMBER(c600_w);
	DECLARE_WRITE8_MEMBER(fgram_w);
	DECLARE_WRITE8_MEMBER(bgram_w);

	TILEMAP_MAPPER_MEMBER(marvins_tx_scan_cols);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	void mainsnk_palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int scrollx, int scrolly );
	void main_map(address_map &map);
	void sound_map(address_map &map);
	void sound_portmap(address_map &map);
};

#endif // MAME_INCLUDES_MAINSNK_H
