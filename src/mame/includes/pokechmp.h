// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_INCLUDES_POKECHMP_H
#define MAME_INCLUDES_POKECHMP_H

#pragma once

#include "machine/gen_latch.h"
#include "emupal.h"
#include "tilemap.h"

class pokechmp_state : public driver_device
{
public:
	pokechmp_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch")
	{ }

	void pokechmp(machine_config &config);

	void init_pokechmp();

private:
	required_shared_ptr<uint8_t> m_videoram;
	tilemap_t *m_bg_tilemap;
	required_shared_ptr<uint8_t> m_spriteram;
	DECLARE_WRITE8_MEMBER(pokechmp_bank_w);
	DECLARE_WRITE8_MEMBER(pokechmp_sound_bank_w);
	DECLARE_WRITE8_MEMBER(pokechmp_sound_w);
	DECLARE_WRITE8_MEMBER(pokechmp_videoram_w);
	DECLARE_WRITE8_MEMBER(pokechmp_flipscreen_w);
	DECLARE_WRITE_LINE_MEMBER(sound_irq);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void video_start() override;
	uint32_t screen_update_pokechmp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	void pokechmp_map(address_map &map);
	void pokechmp_oki_map(address_map &map);
	void pokechmp_sound_map(address_map &map);
};

#endif // MAME_INCLUDES_POKECHMP_H
