// license:BSD-3-Clause
// copyright-holders:Mike Coates
#ifndef MAME_INCLUDES_ZAC2650_H
#define MAME_INCLUDES_ZAC2650_H

#pragma once

#include "machine/s2636.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class zac2650_state : public driver_device
{
public:
	zac2650_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_s2636(*this, "s2636"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_s2636_0_ram(*this, "s2636_0_ram")
	{ }

	void tinvader(machine_config &config);

protected:
	virtual void video_start() override;

private:
	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<s2636_device> m_s2636;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_s2636_0_ram;

	bitmap_ind16 m_bitmap;
	bitmap_ind16 m_spritebitmap;
	int m_CollisionBackground = 0;
	int m_CollisionSprite = 0;
	tilemap_t *m_bg_tilemap = nullptr;

	void tinvader_sound_w(uint8_t data);
	void tinvader_videoram_w(offs_t offset, uint8_t data);
	uint8_t zac_s2636_r(offs_t offset);
	void zac_s2636_w(offs_t offset, uint8_t data);
	uint8_t tinvader_port_0_r();
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	void zac2650_palette(palette_device &palette) const;
	uint32_t screen_update_tinvader(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	int SpriteCollision(int first,int second);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map);
};

#endif // MAME_INCLUDES_ZAC2650_H
