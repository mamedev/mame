// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/*************************************************************************

    Hana Awase

*************************************************************************/

class hanaawas_state : public driver_device
{
public:
	hanaawas_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;

	/* video-related */
	tilemap_t    *m_bg_tilemap;

	/* misc */
	int        m_mux;
	uint8_t hanaawas_input_port_0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void hanaawas_inputs_mux_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void hanaawas_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void hanaawas_colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void irq_ack_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_hanaawas(palette_device &palette);
	uint32_t screen_update_hanaawas(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void hanaawas_portB_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
};
