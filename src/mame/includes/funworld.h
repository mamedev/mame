// license:BSD-3-Clause
// copyright-holders:Roberto Fresca, Peter Ferrie
class funworld_state : public driver_device
{
public:
	funworld_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	tilemap_t *m_bg_tilemap;
	uint8_t questions_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void question_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void funworld_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void funworld_colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void funworld_lamp_a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void funworld_lamp_b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pia1_ca2_w(int state);
	uint8_t funquiz_ay8910_a_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t funquiz_ay8910_b_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t chinatow_r_32f0(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void init_magicd2b();
	void init_magicd2c();
	void init_saloon();
	void init_royalcdc();
	void init_multiwin();
	void init_mongolnw();
	void init_soccernw();
	void init_tabblue();
	void init_dino4();
	void init_ctunk();
	void init_rcdino4();
	void init_rcdinch();
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void video_start_funworld();
	void palette_init_funworld(palette_device &palette);
	void video_start_magicrd2();
	void video_start_chinatow();
	void machine_start_lunapark();
	void machine_reset_lunapark();
	uint32_t screen_update_funworld(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
