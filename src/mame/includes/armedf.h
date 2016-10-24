// license:BSD-3-Clause
// copyright-holders:David Haywood, Phil Stroffolino, Carlos A. Lozano

#include "includes/nb1414m4.h"
#include "machine/gen_latch.h"
#include "video/bufsprite.h"

class armedf_state : public driver_device
{
public:
	armedf_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_extra(*this, "extra"),
		m_nb1414m4(*this, "nb1414m4"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spriteram(*this, "spriteram"),
		m_soundlatch(*this, "soundlatch"),
		m_spr_pal_clut(*this, "spr_pal_clut"),
		m_fg_videoram(*this, "fg_videoram"),
		m_bg_videoram(*this, "bg_videoram")
		{ }

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_extra;
	optional_device<nb1414m4_device> m_nb1414m4;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<buffered_spriteram16_device> m_spriteram;
	required_device<generic_latch_8_device> m_soundlatch;

	/* memory pointers */
	std::unique_ptr<uint8_t[]> m_text_videoram;
	required_shared_ptr<uint16_t> m_spr_pal_clut;
	required_shared_ptr<uint16_t> m_fg_videoram;
	required_shared_ptr<uint16_t> m_bg_videoram;
	uint16_t m_legion_cmd[4]; // legionjb only!

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	tilemap_t  *m_fg_tilemap;
	tilemap_t  *m_tx_tilemap;
	uint16_t   m_scroll_msb;
	uint16_t   m_vreg;
	uint16_t   m_fg_scrollx;
	uint16_t   m_fg_scrolly;
	uint16_t   m_bg_scrollx;
	uint16_t   m_bg_scrolly;
	int      m_scroll_type;
	int      m_sprite_offy;
	int      m_old_mcu_mode;
	int      m_waiting_msb;

	void terraf_io_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void terrafjb_io_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void bootleg_io_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void sound_command_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t soundlatch_clear_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void irq_lv1_ack_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void irq_lv2_ack_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void legionjb_fg_scroll_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t blitter_txram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void blitter_txram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fg_scrollx_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fg_scrolly_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fg_scroll_msb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t nb1414m4_text_videoram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void nb1414m4_text_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t armedf_text_videoram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void armedf_text_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void armedf_fg_videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void armedf_bg_videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void terraf_fg_scrolly_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void terraf_fg_scrollx_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void terraf_fg_scroll_msb_arm_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void armedf_fg_scrollx_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void armedf_fg_scrolly_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void armedf_bg_scrollx_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void armedf_bg_scrolly_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void init_cclimbr2();
	void init_armedf();
	void init_legion();
	void init_terrafu();
	void init_legionjb();
	void init_kozure();
	void init_terraf();
	void init_terrafjb();
	tilemap_memory_index armedf_scan_type1(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	tilemap_memory_index armedf_scan_type2(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	tilemap_memory_index armedf_scan_type3(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	void get_nb1414m4_tx_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_armedf_tx_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void machine_start_armedf();
	void machine_reset_armedf();
	void video_start_terraf();
	void video_start_armedf();
	uint32_t screen_update_armedf(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int priority );
	void armedf_drawgfx(bitmap_ind16 &dest_bmp,const rectangle &clip,gfx_element *gfx,
						uint32_t code,uint32_t color, uint32_t clut,int flipx,int flipy,int offsx,int offsy,
						int transparent_color);
};

class bigfghtr_state : public armedf_state
{
public:
	bigfghtr_state(const machine_config &mconfig, device_type type, const char *tag)
		: armedf_state(mconfig, type, tag),
		m_sharedram(*this, "sharedram") { }

	required_shared_ptr<uint16_t> m_sharedram;

	/* misc */
	int           m_read_latch;
	uint8_t         m_mcu_input_snippet;
	uint8_t         m_mcu_jsr_snippet;

	uint16_t latch_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void sharedram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t sharedram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void init_bigfghtr();
	void machine_start_bigfghtr();
	void machine_reset_bigfghtr();
};
