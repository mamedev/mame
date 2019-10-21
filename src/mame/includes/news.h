// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_INCLUDES_NEWS_H
#define MAME_INCLUDES_NEWS_H

#pragma once

#include "tilemap.h"

class news_state : public driver_device
{
public:
	news_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_bgram(*this, "bgram"),
		m_fgram(*this, "fgram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode")
	{ }

	void news(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_bgram;
	required_shared_ptr<uint8_t> m_fgram;

	/* video-related */
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	int      m_bgpic;

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;

	DECLARE_WRITE8_MEMBER(news_fgram_w);
	DECLARE_WRITE8_MEMBER(news_bgram_w);
	DECLARE_WRITE8_MEMBER(news_bgpic_w);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	uint32_t screen_update_news(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void news_map(address_map &map);
};

#endif // MAME_INCLUDES_NEWS_H
