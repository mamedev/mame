// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Konami Finalizer

***************************************************************************/

class finalizr_state : public driver_device
{
public:
	finalizr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_scroll(*this, "scroll"),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"),
		m_colorram2(*this, "colorram2"),
		m_videoram2(*this, "videoram2"),
		m_spriteram(*this, "spriteram"),
		m_spriteram_2(*this, "spriteram_2")
	{ }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_scroll;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram2;
	required_shared_ptr<uint8_t> m_videoram2;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_spriteram_2;

	/* video-related */
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	int m_spriterambank;
	int m_charbank;

	/* misc */
	int m_T1_line;
	uint8_t m_nmi_enable;
	uint8_t m_irq_enable;

	void finalizr_coin_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void finalizr_flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void finalizr_i8039_irq_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void i8039_irqen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t i8039_T1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void i8039_T0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void finalizr_videoctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_finalizr(palette_device &palette);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_finalizr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void finalizr_scanline(timer_device &timer, void *ptr, int32_t param);
};
