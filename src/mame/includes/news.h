// license:BSD-3-Clause
// copyright-holders:David Haywood

class news_state : public driver_device
{
public:
	news_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_bgram(*this, "bgram"),
		m_fgram(*this, "fgram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_bgram;
	required_shared_ptr<uint8_t> m_fgram;

	/* video-related */
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	int      m_bgpic;
	void news_fgram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void news_bgram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void news_bgpic_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_news(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
};
