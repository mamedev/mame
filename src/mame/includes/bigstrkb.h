// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_INCLUDES_BIGSTRKB_H
#define MAME_INCLUDES_BIGSTRKB_H

#pragma once

#include "emupal.h"
#include "tilemap.h"

class bigstrkb_state : public driver_device
{
public:
	bigstrkb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram2(*this, "videoram2"),
		m_videoram3(*this, "videoram3"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_vidreg1(*this, "vidreg1"),
		m_vidreg2(*this, "vidreg2")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint16_t> m_videoram2;
	required_shared_ptr<uint16_t> m_videoram3;
	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_vidreg1;
	required_shared_ptr<uint16_t> m_vidreg2;

	tilemap_t *m_tilemap;
	tilemap_t *m_tilemap2;
	tilemap_t *m_tilemap3;

	DECLARE_WRITE16_MEMBER(videoram_w);
	DECLARE_WRITE16_MEMBER(videoram2_w);
	DECLARE_WRITE16_MEMBER(videoram3_w);

	TILEMAP_MAPPER_MEMBER(bg_scan);
	TILE_GET_INFO_MEMBER(get_tile_info);
	TILE_GET_INFO_MEMBER(get_tile2_info);
	TILE_GET_INFO_MEMBER(get_tile3_info);

	virtual void video_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );
	void bigstrkb(machine_config &config);
	void bigstrkb_map(address_map &map);
};

#endif // MAME_INCLUDES_BIGSTRKB_H
