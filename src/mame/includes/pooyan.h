// license:BSD-3-Clause
// copyright-holders:Allard van der Bas
#ifndef MAME_INCLUDES_POOYAN_H
#define MAME_INCLUDES_POOYAN_H

#pragma once

#include "emupal.h"
#include "tilemap.h"

class pooyan_state : public driver_device
{
public:
	pooyan_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_spriteram2(*this, "spriteram2")
	{ }

	void pooyan(machine_config &config);

private:
	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_spriteram2;

	/* video-related */
	tilemap_t  *m_bg_tilemap;

	/* misc */
	uint8_t    m_irq_enable;

	DECLARE_WRITE_LINE_MEMBER(irq_enable_w);
	DECLARE_WRITE_LINE_MEMBER(coin_counter_1_w);
	DECLARE_WRITE_LINE_MEMBER(coin_counter_2_w);
	DECLARE_WRITE8_MEMBER(videoram_w);
	DECLARE_WRITE8_MEMBER(colorram_w);
	DECLARE_WRITE_LINE_MEMBER(flipscreen_w);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	virtual void machine_start() override;
	virtual void video_start() override;
	void pooyan_palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );

	DECLARE_WRITE_LINE_MEMBER(vblank_irq);
	void main_map(address_map &map);
};

#endif // MAME_INCLUDES_POOYAN_H
