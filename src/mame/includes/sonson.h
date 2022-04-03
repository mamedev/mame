// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni
/*************************************************************************

    Son Son

*************************************************************************/
#ifndef MAME_INCLUDES_SONSON_H
#define MAME_INCLUDES_SONSON_H

#pragma once

#include "emupal.h"
#include "tilemap.h"

class sonson_state : public driver_device
{
public:
	sonson_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram"),
		m_audiocpu(*this, "audiocpu"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void sonson(machine_config &config);

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_spriteram;

	/* video-related */
	tilemap_t    *m_bg_tilemap = nullptr;

	/* devices */
	required_device<cpu_device> m_audiocpu;
	DECLARE_WRITE_LINE_MEMBER(sh_irqtrigger_w);
	DECLARE_WRITE_LINE_MEMBER(coin1_counter_w);
	DECLARE_WRITE_LINE_MEMBER(coin2_counter_w);
	void sonson_videoram_w(offs_t offset, uint8_t data);
	void sonson_colorram_w(offs_t offset, uint8_t data);
	void sonson_scrollx_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(flipscreen_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void video_start() override;
	void sonson_palette(palette_device &palette) const;
	uint32_t screen_update_sonson(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	void main_map(address_map &map);
	void sound_map(address_map &map);
};

#endif // MAME_INCLUDES_SONSON_H
