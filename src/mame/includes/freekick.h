// license:LGPL-2.1+
// copyright-holders:Tomasz Slanina,David Haywood


class freekick_state : public driver_device
{
public:
	freekick_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_bank1(*this, "bank1"),
		m_bank1d(*this, "bank1d") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;

	/* video-related */
	tilemap_t    *m_freek_tilemap;

	/* misc */
	int        m_inval;
	int        m_outval;
	int        m_cnt;   // used by oigas
	int        m_romaddr;
	int        m_spinner;
	int        m_nmi_en;
	int        m_ff_data;
	void flipscreen_xy_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void coin_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void spinner_select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t spinner_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pbillrd_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void nmi_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void oigas_5_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t oigas_3_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t oigas_2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t freekick_ff_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void freekick_ff_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void freek_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void snd_rom_addr_l_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void snd_rom_addr_h_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t snd_rom_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void init_gigas();
	void init_gigasb();
	void init_pbillrds();
	void get_freek_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void video_start() override;
	void machine_start_pbillrd();
	void machine_reset_freekick();
	void machine_start_freekick();
	void machine_start_oigas();
	void machine_reset_oigas();
	uint32_t screen_update_pbillrd(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_freekick(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_gigas(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void freekick_irqgen(device_t &device);
	void gigas_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void pbillrd_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void freekick_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_memory_bank m_bank1, m_bank1d;
};
