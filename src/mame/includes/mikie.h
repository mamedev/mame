// license:BSD-3-Clause
// copyright-holders:Allard van der Bas
/*************************************************************************

    Mikie

*************************************************************************/
#ifndef MAME_INCLUDES_MIKIE_H
#define MAME_INCLUDES_MIKIE_H

#pragma once

#include "emupal.h"
#include "tilemap.h"

class mikie_state : public driver_device
{
public:
	mikie_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void mikie(machine_config &config);

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_videoram;

	/* video-related */
	tilemap_t  *m_bg_tilemap = nullptr;
	int        m_palettebank = 0;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	uint8_t m_irq_mask = 0;
	uint8_t mikie_sh_timer_r();
	DECLARE_WRITE_LINE_MEMBER(sh_irqtrigger_w);
	DECLARE_WRITE_LINE_MEMBER(coin_counter_1_w);
	DECLARE_WRITE_LINE_MEMBER(coin_counter_2_w);
	DECLARE_WRITE_LINE_MEMBER(irq_mask_w);
	void mikie_videoram_w(offs_t offset, uint8_t data);
	void mikie_colorram_w(offs_t offset, uint8_t data);
	void mikie_palettebank_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(flipscreen_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void mikie_palette(palette_device &palette) const;
	uint32_t screen_update_mikie(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(vblank_irq);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void mikie_map(address_map &map);
	void sound_map(address_map &map);
};

#endif // MAME_INCLUDES_MIKIE_H
