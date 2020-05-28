// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina
#ifndef MAME_INCLUDES_MUSTACHE_H
#define MAME_INCLUDES_MUSTACHE_H

#pragma once

#include "audio/seibu.h"    // for seibu_sound_decrypt on the MAIN cpu (not sound)
#include "machine/timer.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class mustache_state : public driver_device
{
public:
	mustache_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_dswb(*this, "DSWB")
	{ }

	void mustache(machine_config &config);

	void init_mustache();

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;

	required_ioport m_dswb;

	tilemap_t *m_bg_tilemap;
	int m_control_byte;

	DECLARE_WRITE8_MEMBER(videoram_w);
	DECLARE_WRITE8_MEMBER(video_control_w);
	DECLARE_WRITE8_MEMBER(scroll_w);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	virtual void video_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );

	void decrypted_opcodes_map(address_map &map);
	void memmap(address_map &map);
};

#endif // MAME_INCLUDES_MUSTACHE_H
