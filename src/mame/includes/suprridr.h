// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Venture Line Super Rider driver

**************************************************************************/

class suprridr_state : public driver_device
{
public:
	suprridr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_fgram(*this, "fgram"),
		m_bgram(*this, "bgram"),
		m_spriteram(*this, "spriteram") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_fgram;
	required_shared_ptr<uint8_t> m_bgram;
	required_shared_ptr<uint8_t> m_spriteram;

	uint8_t m_nmi_enable;
	uint8_t m_sound_data;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_bg_tilemap_noscroll;
	uint8_t m_flipx;
	uint8_t m_flipy;

	void nmi_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sound_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sound_irq_ack_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void coin_lock_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void flipx_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void flipy_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fgdisable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fgscrolly_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bgscrolly_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bgram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fgram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t sound_data_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	ioport_value control_r(ioport_field &field, void *param);

	void get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info2(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	void main_nmi_gen(device_t &device);
	void delayed_sound_w(void *ptr, int32_t param);

	virtual void machine_start() override;
	virtual void video_start() override;
	void palette_init_suprridr(palette_device &palette);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	int is_screen_flipped();
};
