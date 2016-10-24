// license:BSD-3-Clause
// copyright-holders:David Haywood, Pierpaolo Prazzoli
#include "machine/nmk112.h"

class quizpani_state : public driver_device
{
public:
	quizpani_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_scrollreg(*this, "scrollreg"),
		m_bg_videoram(*this, "bg_videoram"),
		m_txt_videoram(*this, "txt_videoram") { }

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;

	required_shared_ptr<uint16_t> m_scrollreg;
	required_shared_ptr<uint16_t> m_bg_videoram;
	required_shared_ptr<uint16_t> m_txt_videoram;

	tilemap_t *m_bg_tilemap;
	tilemap_t *m_txt_tilemap;
	int m_bgbank;
	int m_txtbank;

	void bg_videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void txt_videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void tilesbank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	tilemap_memory_index bg_scan(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	void bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void txt_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	virtual void video_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
