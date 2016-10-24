// license:BSD-3-Clause
// copyright-holders:Jarek Parchanski
class pingpong_state : public driver_device
{
public:
	pingpong_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	int m_intenable;
	int m_question_addr_high;
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	tilemap_t *m_bg_tilemap;

	void cashquiz_question_bank_high_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void cashquiz_question_bank_low_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void coin_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pingpong_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pingpong_colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_cashquiz();
	void init_merlinmm();
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void video_start() override;
	void palette_init_pingpong(palette_device &palette);
	uint32_t screen_update_pingpong(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void pingpong_interrupt(timer_device &timer, void *ptr, int32_t param);
	void merlinmm_interrupt(timer_device &timer, void *ptr, int32_t param);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );
};
