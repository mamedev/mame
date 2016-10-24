// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli
class rollrace_state : public driver_device
{
public:
	rollrace_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram") { }

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_spriteram;

	tilemap_t *m_fg_tilemap;
	int m_charbank[2];
	int m_bkgpage;
	int m_bkgflip;
	int m_chrbank;
	int m_bkgpen;
	int m_bkgcol;
	int m_flipy;
	int m_flipx;
	int m_spritebank;

	uint8_t m_nmi_mask;
	uint8_t m_sound_nmi_mask;

	uint8_t fake_d800_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void fake_d800_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void nmi_mask_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sound_nmi_mask_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void charbank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bkgpen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void spritebank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void backgroundpage_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void backgroundcolor_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void flipy_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void flipx_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void vram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void cram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void tilemap_refresh_flip();

	void palette_init_rollrace(palette_device &palette);
	virtual void machine_start() override;
	virtual void video_start() override;


	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void vblank_irq(device_t &device);
	void sound_timer_irq(device_t &device);
};
