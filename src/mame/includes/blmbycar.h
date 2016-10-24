// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

    Blomby Car

***************************************************************************/

class blmbycar_state : public driver_device
{
public:
	blmbycar_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_vram_1(*this, "vram_1"),
		m_vram_0(*this, "vram_0"),
		m_scroll_1(*this, "scroll_1"),
		m_scroll_0(*this, "scroll_0"),
		m_spriteram(*this, "spriteram") { }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	/* memory pointers */
	required_shared_ptr<uint16_t> m_vram_1;
	required_shared_ptr<uint16_t> m_vram_0;
	required_shared_ptr<uint16_t> m_scroll_1;
	required_shared_ptr<uint16_t> m_scroll_0;
	required_shared_ptr<uint16_t> m_spriteram;

	/* video-related */
	tilemap_t     *m_tilemap_0;
	tilemap_t     *m_tilemap_1;

	/* input-related */
	uint8_t       m_pot_wheel;    // blmbycar
	int         m_old_val;  // blmbycar
	int         m_retvalue; // waterball

	// common
	void okibank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void vram_0_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void vram_1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	// blmbycar
	void blmbycar_pot_wheel_reset_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void blmbycar_pot_wheel_shift_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t blmbycar_pot_wheel_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t blmbycar_opt_wheel_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);

	// waterball
	uint16_t waterball_unk_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);

	void get_tile_info_0(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info_1(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	void init_blmbycar();
	virtual void video_start() override;
	void machine_start_blmbycar();
	void machine_reset_blmbycar();
	void machine_start_watrball();
	void machine_reset_watrball();

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect );
};
