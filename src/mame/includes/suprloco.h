// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari

#include "machine/gen_latch.h"

class suprloco_state : public driver_device
{
public:
	suprloco_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_soundlatch(*this, "soundlatch"),
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_scrollram(*this, "scrollram"),
		m_decrypted_opcodes(*this, "decrypted_opcodes") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_scrollram;
	required_shared_ptr<uint8_t> m_decrypted_opcodes;

	tilemap_t *m_bg_tilemap;
	int m_control;

	void soundport_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void scrollram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t control_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	void get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	virtual void video_start() override;
	void palette_init_suprloco(palette_device &palette);
	void init_suprloco();

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	inline void draw_pixel(bitmap_ind16 &bitmap,const rectangle &cliprect,int x,int y,int color,int flip);
	void draw_sprite(bitmap_ind16 &bitmap,const rectangle &cliprect,int spr_number);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
};
