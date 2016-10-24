// license:LGPL-2.1+
// copyright-holders:Tomasz Slanina, Pierpaolo Prazzoli
class pitnrun_state : public driver_device
{
public:
	pitnrun_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mcu(*this, "mcu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_videoram2(*this, "videoram2"),
		m_spriteram(*this, "spriteram") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_mcu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_videoram2;
	required_shared_ptr<uint8_t> m_spriteram;

	int m_nmi;
	uint8_t m_fromz80;
	uint8_t m_toz80;
	int m_zaccept;
	int m_zready;
	uint8_t m_portA_in;
	uint8_t m_portA_out;
	int m_address;
	int m_h_heed;
	int m_v_heed;
	int m_ha;
	int m_scroll;
	int m_char_bank;
	int m_color_select;
	std::unique_ptr<bitmap_ind16> m_tmp_bitmap[4];
	tilemap_t *m_bg;
	tilemap_t *m_fg;

	void nmi_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void hflip_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void vflip_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mcu_data_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mcu_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mcu_status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t m68705_portA_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void m68705_portA_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t m68705_portB_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void m68705_portB_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t m68705_portC_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void videoram2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void char_bank_select(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void scroll_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ha_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void h_heed_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void v_heed_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void color_select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void get_tile_info1(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info2(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	void nmi_source(device_t &device);
	void mcu_real_data_r(void *ptr, int32_t param);
	void mcu_real_data_w(void *ptr, int32_t param);
	void mcu_data_real_r(void *ptr, int32_t param);
	void mcu_status_real_w(void *ptr, int32_t param);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_pitnrun(palette_device &palette);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void spotlights();
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );
};
