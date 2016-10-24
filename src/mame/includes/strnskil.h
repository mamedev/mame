// license:BSD-3-Clause
// copyright-holders:Uki
class strnskil_state : public driver_device
{
public:
	strnskil_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_subcpu(*this,"sub"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_xscroll(*this, "xscroll"),
		m_spriteram(*this, "spriteram") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_xscroll;
	required_shared_ptr<uint8_t> m_spriteram;

	uint8_t m_scrl_ctrl;
	tilemap_t *m_bg_tilemap;
	uint8_t m_irq_source;

	void strnskil_irq(timer_device &timer, void *ptr, int32_t param);

	uint8_t strnskil_d800_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t pettanp_protection_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t banbam_protection_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void protection_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void strnskil_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void strnskil_scrl_ctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void init_banbam();
	void init_pettanp();

	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	virtual void machine_start() override;
	virtual void video_start() override;
	void palette_init_strnskil(palette_device &palette);
	uint32_t screen_update_strnskil(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
};
