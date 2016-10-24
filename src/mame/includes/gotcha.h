// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

    Gotcha

*************************************************************************/
#include "sound/okim6295.h"
#include "video/decospr.h"

class gotcha_state : public driver_device
{
public:
	gotcha_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_fgvideoram(*this, "fgvideoram"),
		m_bgvideoram(*this, "bgvideoram"),
		m_spriteram(*this, "spriteram"),
		m_sprgen(*this, "spritegen"),
		m_audiocpu(*this, "audiocpu"),
		m_maincpu(*this, "maincpu"),
		m_oki(*this, "oki"),
		m_gfxdecode(*this, "gfxdecode") { }

	/* memory pointers */
	required_shared_ptr<uint16_t> m_fgvideoram;
	required_shared_ptr<uint16_t> m_bgvideoram;
	required_shared_ptr<uint16_t> m_spriteram;
	optional_device<decospr_device> m_sprgen;

	/* video-related */
	tilemap_t     *m_bg_tilemap;
	tilemap_t     *m_fg_tilemap;
	int         m_banksel;
	int         m_gfxbank[4];
	uint16_t      m_scroll[4];

	/* devices */
	required_device<cpu_device> m_audiocpu;
	void gotcha_lamps_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void gotcha_fgvideoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void gotcha_bgvideoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void gotcha_gfxbank_select_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void gotcha_gfxbank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void gotcha_scroll_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void gotcha_oki_bank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	tilemap_memory_index gotcha_tilemap_scan(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	void fg_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void bg_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_gotcha(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	inline void get_tile_info( tile_data &tileinfo, int tile_index ,uint16_t *vram, int color_offs);
	required_device<cpu_device> m_maincpu;
	required_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
};
