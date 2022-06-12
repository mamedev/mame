// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni
/*************************************************************************

    City Connection

*************************************************************************/
#ifndef MAME_INCLUDES_CITYCON_H
#define MAME_INCLUDES_CITYCON_H

#pragma once

#include "emupal.h"
#include "tilemap.h"

class citycon_state : public driver_device
{
public:
	citycon_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_linecolor(*this, "linecolor"),
		m_spriteram(*this, "spriteram"),
		m_scroll(*this, "scroll"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void init_citycon();
	void citycon(machine_config &config);

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_linecolor;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_scroll;

	/* video-related */
	tilemap_t        *m_bg_tilemap = nullptr;
	tilemap_t        *m_fg_tilemap = nullptr;
	int            m_bg_image = 0;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	uint8_t citycon_in_r();
	uint8_t citycon_irq_ack_r();
	void citycon_videoram_w(offs_t offset, uint8_t data);
	void citycon_linecolor_w(offs_t offset, uint8_t data);
	void citycon_background_w(uint8_t data);
	TILEMAP_MAPPER_MEMBER(citycon_scan);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_citycon(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	inline void changecolor_RRRRGGGGBBBBxxxx( int color, int indx );
	void citycon_map(address_map &map);
	void sound_map(address_map &map);
};

#endif // MAME_INCLUDES_CITYCON_H
