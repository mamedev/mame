// license:BSD-3-Clause
// copyright-holders:Carlos A. Lozano, Uki
/***************************************************************************

    Aeroboto

***************************************************************************/

class aeroboto_state : public driver_device
{
public:
	aeroboto_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_mainram(*this, "mainram"),
		m_videoram(*this, "videoram"),
		m_hscroll(*this, "hscroll"),
		m_tilecolor(*this, "tilecolor"),
		m_spriteram(*this, "spriteram"),
		m_vscroll(*this, "vscroll"),
		m_starx(*this, "starx"),
		m_stary(*this, "stary"),
		m_bgcolor(*this, "bgcolor"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_mainram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_hscroll;
	required_shared_ptr<uint8_t> m_tilecolor;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_vscroll;
	required_shared_ptr<uint8_t> m_starx;
	required_shared_ptr<uint8_t> m_stary;
	required_shared_ptr<uint8_t> m_bgcolor;

	/* stars layout */
	uint8_t * m_stars_rom;
	int     m_stars_length;

	/* video-related */
	tilemap_t *m_bg_tilemap;
	int     m_charbank;
	int     m_starsoff;
	int     m_sx;
	int     m_sy;
	uint8_t   m_ox;
	uint8_t   m_oy;

	/* misc */
	int m_count;
	int m_disable_irq;
	uint8_t aeroboto_201_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t aeroboto_irq_ack_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t aeroboto_2973_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void aeroboto_1a2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t aeroboto_in0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void aeroboto_3000_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void aeroboto_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void aeroboto_tilecolor_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_aeroboto(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void aeroboto_interrupt(device_t &device);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
