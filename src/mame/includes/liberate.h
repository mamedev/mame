// license:BSD-3-Clause
// copyright-holders:Bryan McPhail

#include "machine/gen_latch.h"

class liberate_state : public driver_device
{
public:
	liberate_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_bg_vram(*this, "bg_vram"),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_scratchram(*this, "scratchram"),
		m_decrypted_opcodes(*this, "decrypted_opcodes"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch") { }

	optional_shared_ptr<uint8_t> m_bg_vram; /* prosport */
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	optional_shared_ptr<uint8_t> m_scratchram;
	optional_shared_ptr<uint8_t> m_decrypted_opcodes;

	uint8_t *m_fg_gfx;   /* prosoccr */
	std::unique_ptr<uint8_t[]> m_charram;   /* prosoccr */
	uint8_t m_io_ram[16];

	int m_bank;
	int m_latch;
	uint8_t m_gfx_rom_readback;
	int m_background_color;
	int m_background_disable;

	tilemap_t *m_back_tilemap;
	tilemap_t *m_fix_tilemap;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	uint8_t deco16_bank_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t deco16_io_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void deco16_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t prosoccr_bank_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t prosoccr_charram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void prosoccr_charram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void prosoccr_char_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void prosoccr_io_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t prosport_charram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void prosport_charram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void deco16_io_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void prosoccr_io_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void prosport_io_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void liberate_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void liberate_colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void prosport_bg_vram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_yellowcb();
	void init_liberate();
	void init_prosport();
	tilemap_memory_index back_scan(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	tilemap_memory_index fix_scan(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	void get_back_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fix_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void prosport_get_back_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void machine_start_liberate();
	void machine_reset_liberate();
	void video_start_liberate();
	void palette_init_liberate(palette_device &palette);
	void video_start_prosport();
	void video_start_boomrang();
	void video_start_prosoccr();
	uint32_t screen_update_liberate(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_prosport(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_boomrang(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_prosoccr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void deco16_interrupt(device_t &device);
	void liberate_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void prosport_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void boomrang_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int pri );
	void prosoccr_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
};
