// license:BSD-3-Clause
// copyright-holders:David Haywood
/*************************************************************************

    Gumbo - Miss Bingo - Miss Puzzle

*************************************************************************/
#ifndef MAME_INCLUDES_GUMBO_H
#define MAME_INCLUDES_GUMBO_H

#pragma once

class gumbo_state : public driver_device
{
public:
	gumbo_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_bg_videoram(*this, "bg_videoram"),
		m_fg_videoram(*this, "fg_videoram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode")
	{ }

	void mspuzzle(machine_config &config);
	void dblpoint(machine_config &config);
	void gumbo(machine_config &config);

protected:
	virtual void video_start() override;

private:
	/* memory pointers */
	required_shared_ptr<uint16_t> m_bg_videoram;
	required_shared_ptr<uint16_t> m_fg_videoram;

	/* video-related */
	tilemap_t    *m_bg_tilemap;
	tilemap_t    *m_fg_tilemap;

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;

	DECLARE_WRITE16_MEMBER(gumbo_bg_videoram_w);
	DECLARE_WRITE16_MEMBER(gumbo_fg_videoram_w);
	TILE_GET_INFO_MEMBER(get_gumbo_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_gumbo_fg_tile_info);
	uint32_t screen_update_gumbo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void dblpoint_map(address_map &map);
	void gumbo_map(address_map &map);
	void mspuzzle_map(address_map &map);
};

#endif // MAME_INCLUDES_GUMBO_H
