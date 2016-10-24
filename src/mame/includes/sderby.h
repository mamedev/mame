// license:BSD-3-Clause
// copyright-holders:David Haywood, Roberto Fresca
class sderby_state : public driver_device
{
public:
	sderby_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_md_videoram(*this, "md_videoram"),
		m_fg_videoram(*this, "fg_videoram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint16_t> m_md_videoram;
	required_shared_ptr<uint16_t> m_fg_videoram;
	required_shared_ptr<uint16_t> m_spriteram;

	tilemap_t *m_tilemap;
	tilemap_t *m_md_tilemap;
	tilemap_t *m_fg_tilemap;

	uint16_t m_scroll[6];
	uint16_t sderby_input_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t sderbya_input_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t roulette_input_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t rprot_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void rprot_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void sderby_out_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void scmatto_out_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void roulette_out_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void sderby_videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void sderby_md_videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void sderby_fg_videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void sderby_scroll_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void get_sderby_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_sderby_md_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_sderby_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void video_start() override;
	uint32_t screen_update_sderby(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_pmroulet(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect,int codeshift);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
