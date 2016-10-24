// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari, Curt Coder
class lvcards_state : public driver_device
{
public:
	lvcards_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode") { }

	uint8_t m_payout;
	uint8_t m_pulse;
	uint8_t m_result;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	tilemap_t *m_bg_tilemap;
	void control_port_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void control_port_2a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t payout_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void lvcards_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void lvcards_colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void video_start() override;
	void palette_init_lvcards(palette_device &palette);
	void machine_start_lvpoker();
	void machine_reset_lvpoker();
	void palette_init_ponttehk(palette_device &palette);
	uint32_t screen_update_lvcards(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
};
