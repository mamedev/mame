// license:BSD-3-Clause
// copyright-holders:Allard van der Bas
#ifndef MAME_INCLUDES_SHAOLINS_H
#define MAME_INCLUDES_SHAOLINS_H

#pragma once

#include "machine/timer.h"
#include "emupal.h"
#include "tilemap.h"

class shaolins_state : public driver_device
{
public:
	shaolins_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_screen(*this,"screen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spriteram(*this, "spriteram"),
		m_spriteram2(*this, "spriteram2"),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram")
	{ }

	void shaolins(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_spriteram2;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_videoram;

	int m_palettebank = 0;
	tilemap_t *m_bg_tilemap = nullptr;
	uint8_t m_nmi_enable = 0;

	void videoram_w(offs_t offset, uint8_t data);
	void colorram_w(offs_t offset, uint8_t data);
	void palettebank_w(uint8_t data);
	void scroll_w(uint8_t data);
	void nmi_w(uint8_t data);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	virtual void video_start() override;
	void shaolins_palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	TIMER_DEVICE_CALLBACK_MEMBER(interrupt);
	void shaolins_map(address_map &map);
};

#endif // MAME_INCLUDES_SHAOLINS_H
