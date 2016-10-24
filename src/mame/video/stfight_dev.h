// license:BSD-3-Clause
// copyright-holders:Mark McDougall, David Haywood

#pragma once

#ifndef __STFIGHT_VIDEO__
#define __STFIGHT_VIDEO__



extern const device_type STFIGHT_VIDEO;

#define MCFG_STFIGHT_VIDEO_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, STFIGHT_VIDEO, 0)


class stfight_video_device :  public device_t
{
public:
	// construction/destruction
	stfight_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);


	void stfight_text_char_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void stfight_sprite_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void stfight_vh_latch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	uint32_t screen_update_stfight(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// devices
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;

	// region pointers
	required_region_ptr<uint8_t> m_tx_clut;
	required_region_ptr<uint8_t> m_fg_clut;
	required_region_ptr<uint8_t> m_bg_clut;
	required_region_ptr<uint8_t> m_spr_clut;
	required_region_ptr<uint8_t> m_fgmap;
	required_region_ptr<uint8_t> m_bgmap;

	// memory pointers
	required_shared_ptr<uint8_t> m_vregs;
	required_shared_ptr<uint8_t> m_sprite_ram;
	required_shared_ptr<uint8_t> m_txram;

	// tilemaps
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_tx_tilemap;

	tilemap_memory_index fg_scan(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	tilemap_memory_index bg_scan(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);

	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tx_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	// internal variables
	int m_sprite_base;

	// rendering / mixing
	bitmap_ind16 m_temp_bitmap;
	bitmap_ind16 m_temp_sprite_bitmap;
	void mix_txlayer(screen_device &screen, bitmap_ind16 &bitmap, bitmap_ind16 &bitmap2, const rectangle &cliprect, uint8_t* clut, int base, int mask, int condition, bool realcheck);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};

#endif
