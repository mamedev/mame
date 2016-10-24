// license:BSD-3-Clause
// copyright-holders:Chris Hardy
/*************************************************************************

    Megazone

*************************************************************************/

class megazone_state : public driver_device
{
public:
	megazone_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_scrolly(*this, "scrolly"),
		m_scrollx(*this, "scrollx"),
		m_videoram(*this, "videoram"),
		m_videoram2(*this, "videoram2"),
		m_colorram(*this, "colorram"),
		m_colorram2(*this, "colorram2"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_daccpu(*this, "daccpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_scrolly;
	required_shared_ptr<uint8_t> m_scrollx;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_videoram2;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_colorram2;
	required_shared_ptr<uint8_t> m_spriteram;

	/* video-related */
	std::unique_ptr<bitmap_ind16>   m_tmpbitmap;
	int           m_flipscreen;

	/* misc */
	int           m_i8039_status;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_daccpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	uint8_t         m_irq_mask;
	void megazone_i8039_irq_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void i8039_irqen_and_status_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void megazone_coin_counter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void irq_mask_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void megazone_flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t megazone_port_a_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void megazone_port_b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_megazone(palette_device &palette);
	uint32_t screen_update_megazone(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vblank_irq(device_t &device);
};
