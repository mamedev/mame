// license:BSD-3-Clause
// copyright-holders:Luca Elia
#include "machine/tmp68301.h"

class realbrk_state : public driver_device
{
public:
	realbrk_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_tmp68301(*this, "tmp68301"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_spriteram(*this, "spriteram"),
		m_vram_0(*this, "vram_0"),
		m_vram_1(*this, "vram_1"),
		m_vram_2(*this, "vram_2"),
		m_vregs(*this, "vregs"),
		m_dsw_select(*this, "dsw_select"),
		m_backup_ram(*this, "backup_ram"),
		m_vram_0ras(*this, "vram_0ras"),
		m_vram_1ras(*this, "vram_1ras") { }

	required_device<cpu_device> m_maincpu;
	required_device<tmp68301_device> m_tmp68301;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_vram_0;
	required_shared_ptr<uint16_t> m_vram_1;
	required_shared_ptr<uint16_t> m_vram_2;
	required_shared_ptr<uint16_t> m_vregs;
	optional_shared_ptr<uint16_t> m_dsw_select;
	optional_shared_ptr<uint16_t> m_backup_ram;
	optional_shared_ptr<uint16_t> m_vram_0ras;
	optional_shared_ptr<uint16_t> m_vram_1ras;

	std::unique_ptr<bitmap_ind16> m_tmpbitmap0;
	std::unique_ptr<bitmap_ind16> m_tmpbitmap1;
	int m_disable_video;
	tilemap_t *m_tilemap_0;
	tilemap_t *m_tilemap_1;
	tilemap_t *m_tilemap_2;

	// common
	void vram_0_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void vram_1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void vram_2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void vregs_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	// realbrk and/or dai2kaku
	uint16_t realbrk_dsw_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void realbrk_flipscreen_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void dai2kaku_flipscreen_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	// pkgnsh and/or pkgnshdx
	uint16_t pkgnsh_input_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t pkgnshdx_input_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t backup_ram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t backup_ram_dx_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void backup_ram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	void get_tile_info_0(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info_1(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info_2(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	virtual void video_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_dai2kaku(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);
	void dai2kaku_draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect, int layer);

	void interrupt(device_t &device);
};
