// license:BSD-3-Clause
// copyright-holders:Richard Davies
/*************************************************************************

    Exed Exes

*************************************************************************/

#include "video/bufsprite.h"

class exedexes_state : public driver_device
{
public:
	exedexes_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_spriteram(*this, "spriteram") ,
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_nbg_yscroll(*this, "nbg_yscroll"),
		m_nbg_xscroll(*this, "nbg_xscroll"),
		m_bg_scroll(*this, "bg_scroll"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_device<buffered_spriteram8_device> m_spriteram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_nbg_yscroll;
	required_shared_ptr<uint8_t> m_nbg_xscroll;
	required_shared_ptr<uint8_t> m_bg_scroll;

	/* video-related */
	tilemap_t        *m_bg_tilemap;
	tilemap_t        *m_fg_tilemap;
	tilemap_t        *m_tx_tilemap;
	int            m_chon;
	int            m_objon;
	int            m_sc1on;
	int            m_sc2on;

	void exedexes_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void exedexes_colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void exedexes_c804_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void exedexes_gfxctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tx_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	tilemap_memory_index exedexes_bg_tilemap_scan(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	tilemap_memory_index exedexes_fg_tilemap_scan(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_exedexes(palette_device &palette);
	uint32_t screen_update_exedexes(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void exedexes_scanline(timer_device &timer, void *ptr, int32_t param);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int priority );
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
