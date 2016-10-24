// license:BSD-3-Clause
// copyright-holders:Luca Elia
#include "sound/okim6295.h"

class paradise_state : public driver_device
{
public:
	paradise_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_oki2(*this, "oki2"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_vram_0(*this, "vram_0"),
		m_vram_1(*this, "vram_1"),
		m_vram_2(*this, "vram_2"),
		m_videoram(*this, "videoram"),
		m_paletteram(*this, "paletteram"),
		m_spriteram(*this, "spriteram") { }

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<okim6295_device> m_oki2;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_vram_0;
	required_shared_ptr<uint8_t> m_vram_1;
	required_shared_ptr<uint8_t> m_vram_2;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_paletteram;
	required_shared_ptr<uint8_t> m_spriteram;

	/* video-related */
	tilemap_t *m_tilemap_0;
	tilemap_t *m_tilemap_1;
	tilemap_t *m_tilemap_2;

	bitmap_ind16 m_tmpbitmap;
	uint8_t m_palbank;
	uint8_t m_priority;
	uint8_t m_pixbank;
	int m_sprite_inc;
	int m_irq_count;

	// common
	void rombank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void palette_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void palbank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void vram_0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void vram_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void vram_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pixmap_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void priority_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	// paradise specific
	void paradise_okibank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	// torus specific
	void torus_coin_counter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	// tgtball specific
	void tgtball_flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void init_torus();
	void init_paradise();
	void init_tgtball();

	void get_tile_info_0(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info_1(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info_2(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	uint32_t screen_update_paradise(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_torus(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_madball(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void irq(device_t &device);

	void update_pix_palbank();
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
};
