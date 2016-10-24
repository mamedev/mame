// license:BSD-3-Clause
// copyright-holders:Mike Coates
#include "machine/s2636.h"

class zac2650_state : public driver_device
{
public:
	zac2650_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_s2636(*this, "s2636"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_s2636_0_ram(*this, "s2636_0_ram") { }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<s2636_device> m_s2636;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_s2636_0_ram;

	bitmap_ind16 m_bitmap;
	bitmap_ind16 m_spritebitmap;
	int m_CollisionBackground;
	int m_CollisionSprite;
	tilemap_t *m_bg_tilemap;

	void tinvader_sound_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tinvader_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t zac_s2636_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void zac_s2636_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t tinvader_port_0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void video_start() override;
	void palette_init_zac2650(palette_device &palette);
	uint32_t screen_update_tinvader(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	int SpriteCollision(int first,int second);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
};
