// license:BSD-3-Clause
// copyright-holders:Uki

/*************************************************************************

    Himeshikibu

*************************************************************************/

#include "machine/gen_latch.h"

class himesiki_state : public driver_device
{
public:
	himesiki_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_bg_ram(*this, "bg_ram"),
		m_spriteram_p103a(*this, "sprram_p103a"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_bg_ram;
	required_shared_ptr<uint8_t> m_spriteram_p103a;
	required_shared_ptr<uint8_t> m_spriteram;

	/* video-related */
	tilemap_t    *m_bg_tilemap;
	int          m_scrollx[2];
	int          m_scrolly;

	int        m_flipscreen;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	void himesiki_rombank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void himesiki_sound_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void himesiki_bg_ram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void himesiki_scrollx_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void himesiki_scrolly_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_himesiki(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void himesiki_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
};
