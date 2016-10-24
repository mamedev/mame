// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina, Angelo Salese, hap, David Haywood

#pragma once

#ifndef __AIRRAID_VIDEO__
#define __AIRRAID_VIDEO__



extern const device_type AIRRAID_VIDEO;

#define MCFG_AIRRAID_VIDEO_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, AIRRAID_VIDEO, 0)


class airraid_video_device :  public device_t
/*  public device_video_interface */
{
public:
	// construction/destruction
	airraid_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void txram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void vregs_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void layer_enable_w(uint8_t enable);

	uint32_t screen_update_airraid(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

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
	required_shared_ptr<uint8_t> m_sprite_ram;
	required_shared_ptr<uint8_t> m_txram;
	required_shared_ptr<uint8_t> m_vregs;

	// tilemaps
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_tx_tilemap;

	tilemap_memory_index bg_scan(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	tilemap_memory_index fg_scan(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);

	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_cstx_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	// internal variables
	uint16_t m_hw;

	// rendering / mixing
	bitmap_ind16 m_temp_bitmap;
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void mix_layer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t* clut, int base);
};

#endif
