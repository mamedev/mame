// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "sound/okim6295.h"
#include "cpu/pic16c5x/pic16c5x.h"

class drgnmst_state : public driver_device
{
public:
	drgnmst_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_vidregs(*this, "vidregs"),
		m_fg_videoram(*this, "fg_videoram"),
		m_bg_videoram(*this, "bg_videoram"),
		m_md_videoram(*this, "md_videoram"),
		m_rowscrollram(*this, "rowscrollram"),
		m_vidregs2(*this, "vidregs2"),
		m_spriteram(*this, "spriteram"),
			m_oki_1(*this, "oki1"),
			m_oki_2(*this, "oki2") ,
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<uint16_t> m_vidregs;
	required_shared_ptr<uint16_t> m_fg_videoram;
	required_shared_ptr<uint16_t> m_bg_videoram;
	required_shared_ptr<uint16_t> m_md_videoram;
	required_shared_ptr<uint16_t> m_rowscrollram;
	required_shared_ptr<uint16_t> m_vidregs2;
	required_shared_ptr<uint16_t> m_spriteram;

	/* video-related */
	tilemap_t     *m_bg_tilemap;
	tilemap_t     *m_fg_tilemap;
	tilemap_t     *m_md_tilemap;

	/* misc */
	uint16_t      m_snd_command;
	uint16_t      m_snd_flag;
	uint8_t       m_oki_control;
	uint8_t       m_oki_command;
	uint8_t       m_pic16c5x_port0;
	uint8_t       m_oki0_bank;
	uint8_t       m_oki1_bank;

	/* devices */
	required_device<okim6295_device> m_oki_1;
	required_device<okim6295_device> m_oki_2;
	void drgnmst_coin_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void drgnmst_snd_command_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void drgnmst_snd_flag_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t pic16c5x_port0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t drgnmst_snd_command_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t drgnmst_snd_flag_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void drgnmst_pcm_banksel_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void drgnmst_oki_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void drgnmst_snd_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	int PIC16C5X_T0_clk_r();
	void drgnmst_fg_videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void drgnmst_bg_videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void drgnmst_md_videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void init_drgnmst();
	void get_drgnmst_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_drgnmst_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_drgnmst_md_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	tilemap_memory_index drgnmst_fg_tilemap_scan_cols(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	tilemap_memory_index drgnmst_md_tilemap_scan_cols(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	tilemap_memory_index drgnmst_bg_tilemap_scan_cols(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_drgnmst(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap,const rectangle &cliprect );
	uint8_t drgnmst_asciitohex( uint8_t data );
	required_device<cpu_device> m_maincpu;
	required_device<pic16c55_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
