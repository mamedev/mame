// license:BSD-3-Clause
// copyright-holders:Luca Elia
#include "machine/nmk112.h"

class powerins_state : public driver_device
{
public:
	powerins_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_vctrl_0(*this, "vctrl_0"),
		m_vram_0(*this, "vram_0"),
		m_vram_1(*this, "vram_1"),
		m_spriteram(*this, "spriteram") { }


	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint16_t> m_vctrl_0;
	required_shared_ptr<uint16_t> m_vram_0;
	required_shared_ptr<uint16_t> m_vram_1;
	required_shared_ptr<uint16_t> m_spriteram;

	tilemap_t *m_tilemap_0;
	tilemap_t *m_tilemap_1;
	int m_tile_bank;

	void powerinsa_okibank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tilebank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void vram_0_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void vram_1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t powerinsb_fake_ym2203_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	void machine_start_powerinsa();

	void get_tile_info_0(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info_1(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	tilemap_memory_index get_memory_offset_0(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);

	virtual void video_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);
};
