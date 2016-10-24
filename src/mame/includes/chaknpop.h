// license:BSD-3-Clause
// copyright-holders:BUT


#define MCU_INITIAL_SEED    0x81


class chaknpop_state : public driver_device
{
public:
	chaknpop_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_mcu_ram(*this, "mcu_ram"),
		m_tx_ram(*this, "tx_ram"),
		m_attr_ram(*this, "attr_ram"),
		m_spr_ram(*this, "spr_ram") { }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_mcu_ram;
	required_shared_ptr<uint8_t> m_tx_ram;
	required_shared_ptr<uint8_t> m_attr_ram;
	required_shared_ptr<uint8_t> m_spr_ram;

	/* mcu-related */
	uint8_t m_mcu_seed;
	uint8_t m_mcu_select;
	uint8_t m_mcu_result;


	/* video-related */
	tilemap_t  *m_tx_tilemap;
	uint8_t    *m_vram1;
	uint8_t    *m_vram2;
	uint8_t    *m_vram3;
	uint8_t    *m_vram4;
	uint8_t    m_gfxmode;
	uint8_t    m_flip_x;
	uint8_t    m_flip_y;

	void coinlock_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mcu_port_a_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t mcu_port_b_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t mcu_port_c_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mcu_port_a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mcu_port_b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mcu_port_c_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t gfxmode_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void gfxmode_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void txram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void attrram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void unknown_port_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void unknown_port_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void get_tx_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_chaknpop(palette_device &palette);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void tx_tilemap_mark_all_dirty();
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_bitmap(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void mcu_update_seed(uint8_t data);
};
