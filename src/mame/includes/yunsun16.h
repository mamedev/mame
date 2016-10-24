// license:BSD-3-Clause
// copyright-holders:Luca Elia

/*************************************************************************

    Yun Sung 16 Bit Games

*************************************************************************/

#include "machine/gen_latch.h"

class yunsun16_state : public driver_device
{
public:
	yunsun16_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_vram_0(*this, "vram_0"),
		m_vram_1(*this, "vram_1"),
		m_scrollram_0(*this, "scrollram_0"),
		m_scrollram_1(*this, "scrollram_1"),
		m_priorityram(*this, "priorityram"),
		m_spriteram(*this, "spriteram") { }

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_device<generic_latch_8_device> m_soundlatch; // not shocking

	/* memory pointers */
	required_shared_ptr<uint16_t> m_vram_0;
	required_shared_ptr<uint16_t> m_vram_1;
	required_shared_ptr<uint16_t> m_scrollram_0;
	required_shared_ptr<uint16_t> m_scrollram_1;
	required_shared_ptr<uint16_t> m_priorityram;
	required_shared_ptr<uint16_t> m_spriteram;

	/* other video-related elements */
	tilemap_t     *m_tilemap_0;
	tilemap_t     *m_tilemap_1;
	int         m_sprites_scrolldx;
	int         m_sprites_scrolldy;

	void sound_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void magicbub_sound_command_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void vram_0_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void vram_1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void init_magicbub();
	void machine_start_shocking();
	void machine_reset_shocking();
	tilemap_memory_index tilemap_scan_pages(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	void get_tile_info_0(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info_1(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_yunsun16(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect );
};
