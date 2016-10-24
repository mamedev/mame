// license:BSD-3-Clause
// copyright-holders:Carlos A. Lozano

#include "machine/gen_latch.h"
#include "video/bufsprite.h"

class terracre_state : public driver_device
{
public:
	terracre_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spriteram(*this, "spriteram"),
		m_soundlatch(*this, "soundlatch"),
		m_bg_videoram(*this, "bg_videoram"),
		m_fg_videoram(*this, "fg_videoram") { }

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<buffered_spriteram16_device> m_spriteram;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint16_t> m_bg_videoram;
	required_shared_ptr<uint16_t> m_fg_videoram;

	const uint16_t *m_mpProtData;
	uint8_t m_mAmazonProtCmd;
	uint8_t m_mAmazonProtReg[6];
	uint16_t m_xscroll;
	uint16_t m_yscroll;
	tilemap_t *m_background;
	tilemap_t *m_foreground;
	void amazon_sound_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t soundlatch_clear_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint16_t amazon_protection_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void amazon_protection_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void amazon_background_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void amazon_foreground_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void amazon_flipscreen_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void amazon_scrolly_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void amazon_scrollx_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void init_amazon();
	void init_amatelas();
	void init_horekid();
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void video_start() override;
	void palette_init_terracre(palette_device &palette);
	void machine_start_amazon();
	uint32_t screen_update_amazon(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );
};
