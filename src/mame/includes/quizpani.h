// license:BSD-3-Clause
// copyright-holders:David Haywood, Pierpaolo Prazzoli
#ifndef MAME_INCLUDES_QUIZPANI_H
#define MAME_INCLUDES_QUIZPANI_H

#pragma once

#include "machine/nmk112.h"
#include "tilemap.h"

class quizpani_state : public driver_device
{
public:
	quizpani_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_scrollreg(*this, "scrollreg"),
		m_bg_videoram(*this, "bg_videoram"),
		m_txt_videoram(*this, "txt_videoram")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;

	required_shared_ptr<uint16_t> m_scrollreg;
	required_shared_ptr<uint16_t> m_bg_videoram;
	required_shared_ptr<uint16_t> m_txt_videoram;

	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_txt_tilemap = nullptr;
	int m_bgbank = 0;
	int m_txtbank = 0;

	void bg_videoram_w(offs_t offset, uint16_t data);
	void txt_videoram_w(offs_t offset, uint16_t data);
	void tilesbank_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	TILEMAP_MAPPER_MEMBER(bg_scan);
	TILE_GET_INFO_MEMBER(bg_tile_info);
	TILE_GET_INFO_MEMBER(txt_tile_info);

	virtual void video_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void quizpani(machine_config &config);
	void quizpani_map(address_map &map);
};

#endif // MAME_INCLUDES_QUIZPANI_H
