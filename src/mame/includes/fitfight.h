// license:BSD-3-Clause
// copyright-holders:David Haywood

class fitfight_state : public driver_device
{
public:
	fitfight_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_fof_100000(*this, "fof_100000"),
		m_fof_600000(*this, "fof_600000"),
		m_fof_700000(*this, "fof_700000"),
		m_fof_800000(*this, "fof_800000"),
		m_fof_900000(*this, "fof_900000"),
		m_fof_a00000(*this, "fof_a00000"),
		m_fof_bak_tileram(*this, "fof_bak_tileram"),
		m_fof_mid_tileram(*this, "fof_mid_tileram"),
		m_fof_txt_tileram(*this, "fof_txt_tileram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<uint16_t> m_fof_100000;
	required_shared_ptr<uint16_t> m_fof_600000;
	required_shared_ptr<uint16_t> m_fof_700000;
	required_shared_ptr<uint16_t> m_fof_800000;
	required_shared_ptr<uint16_t> m_fof_900000;
	required_shared_ptr<uint16_t> m_fof_a00000;
	required_shared_ptr<uint16_t> m_fof_bak_tileram;
	required_shared_ptr<uint16_t> m_fof_mid_tileram;
	required_shared_ptr<uint16_t> m_fof_txt_tileram;
	required_shared_ptr<uint16_t> m_spriteram;

	/* video-related */
	tilemap_t  *m_fof_bak_tilemap;
	tilemap_t  *m_fof_mid_tilemap;
	tilemap_t  *m_fof_txt_tilemap;

	/* misc */
	int      m_bbprot_kludge;
	uint16_t   m_fof_700000_data;
	uint16_t fitfight_700000_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t histryma_700000_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t bbprot_700000_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void fitfight_700000_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t snd_porta_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t snd_portb_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t snd_portc_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void snd_porta_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void snd_portb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void snd_portc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fof_bak_tileram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void fof_mid_tileram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void fof_txt_tileram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	uint16_t hotmindff_unk_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void init_hotmindff();
	void init_fitfight();
	void init_histryma();
	void init_bbprot();
	void get_fof_bak_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fof_mid_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fof_txt_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_fitfight(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void snd_irq(device_t &device);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int layer );
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
