// license:BSD-3-Clause
// copyright-holders:David Haywood

/*************************************************************************

    Macross Plus

*************************************************************************/

#include "machine/gen_latch.h"

class macrossp_state : public driver_device
{
public:
	macrossp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_scra_videoram(*this, "scra_videoram"),
		m_scra_linezoom(*this, "scra_linezoom"),
		m_scra_videoregs(*this, "scra_videoregs"),

		m_scrb_videoram(*this, "scrb_videoram"),
		m_scrb_linezoom(*this, "scrb_linezoom"),
		m_scrb_videoregs(*this, "scrb_videoregs"),

		m_scrc_videoram(*this, "scrc_videoram"),
		m_scrc_linezoom(*this, "scrc_linezoom"),
		m_scrc_videoregs(*this, "scrc_videoregs"),

		m_text_videoram(*this, "text_videoram"),
		m_text_linezoom(*this, "text_linezoom"),
		m_text_videoregs(*this, "text_videoregs"),

		m_mainram(*this, "mainram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch")
	{
	}

	/* memory pointers */
	required_shared_ptr<uint32_t> m_spriteram;
	required_shared_ptr<uint32_t> m_scra_videoram;
	required_shared_ptr<uint32_t> m_scra_linezoom;
	required_shared_ptr<uint32_t> m_scra_videoregs;
	required_shared_ptr<uint32_t> m_scrb_videoram;
	required_shared_ptr<uint32_t> m_scrb_linezoom;
	required_shared_ptr<uint32_t> m_scrb_videoregs;
	required_shared_ptr<uint32_t> m_scrc_videoram;
	required_shared_ptr<uint32_t> m_scrc_linezoom;
	required_shared_ptr<uint32_t> m_scrc_videoregs;
	required_shared_ptr<uint32_t> m_text_videoram;
	required_shared_ptr<uint32_t> m_text_linezoom;
	required_shared_ptr<uint32_t> m_text_videoregs;
	required_shared_ptr<uint32_t> m_mainram;
	std::unique_ptr<uint32_t[]>         m_spriteram_old;
	std::unique_ptr<uint32_t[]>         m_spriteram_old2;

	/* video-related */
	tilemap_t  *m_scra_tilemap;
	tilemap_t  *m_scrb_tilemap;
	tilemap_t  *m_scrc_tilemap;
	tilemap_t  *m_text_tilemap;

	/* misc */
	int              m_sndpending;
	int              m_snd_toggle;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_16_device> m_soundlatch;

	uint32_t macrossp_soundstatus_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void macrossp_soundcmd_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint16_t macrossp_soundcmd_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void palette_fade_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void macrossp_speedup_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void macrossp_scra_videoram_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void macrossp_scrb_videoram_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void macrossp_scrc_videoram_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void macrossp_text_videoram_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void init_quizmoon();
	void init_macrossp();
	void get_macrossp_scra_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_macrossp_scrb_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_macrossp_scrc_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_macrossp_text_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_macrossp(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void screen_eof_macrossp(screen_device &screen, bool state);
	void draw_sprites(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_layer(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int layer, int linem, int pri);
	void irqhandler(int state);
};
