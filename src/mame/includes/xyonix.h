// license:BSD-3-Clause
// copyright-holders:David Haywood
class xyonix_state : public driver_device
{
public:
	xyonix_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_vidram(*this, "vidram") { }

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;

	required_shared_ptr<uint8_t> m_vidram;

	tilemap_t *m_tilemap;

	int m_e0_data;
	int m_credits;
	int m_coins;
	int m_prev_coin;

	void irqack_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t io_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void io_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void vidram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	virtual void machine_start() override;
	virtual void video_start() override;
	void get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void palette_init_xyonix(palette_device &palette);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void handle_coins(int coin);
};
