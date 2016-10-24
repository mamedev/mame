// license:BSD-3-Clause
// copyright-holders:Luca Elia
/*************************************************************************

    Metal Clash

*************************************************************************/

class metlclsh_state : public driver_device
{
public:
	metlclsh_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_fgram(*this, "fgram"),
		m_spriteram(*this, "spriteram"),
		m_bgram(*this, "bgram"),
		m_scrollx(*this, "scrollx"),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_fgram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_bgram;
	required_shared_ptr<uint8_t> m_scrollx;
	std::unique_ptr<uint8_t[]>        m_otherram;

	/* video-related */
	tilemap_t      *m_bg_tilemap;
	tilemap_t      *m_fg_tilemap;
	uint8_t          m_write_mask;
	uint8_t          m_gfxbank;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	void metlclsh_cause_irq(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void metlclsh_ack_nmi(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void metlclsh_cause_nmi2(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void metlclsh_ack_irq2(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void metlclsh_ack_nmi2(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void metlclsh_flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void metlclsh_rambank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void metlclsh_gfxbank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void metlclsh_bgram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void metlclsh_fgram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void coin_inserted(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);
	tilemap_memory_index metlclsh_bgtilemap_scan(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_metlclsh(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
};
