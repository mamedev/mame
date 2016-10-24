// license:BSD-3-Clause
// copyright-holders:Dan Boris
/***************************************************************************

    Sun Electronics Arabian hardware

    driver by Dan Boris

***************************************************************************/

class arabian_state : public driver_device
{
public:
	arabian_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_custom_cpu_ram(*this, "custom_cpu_ram"),
		m_blitter(*this, "blitter"),
		m_maincpu(*this, "maincpu"),
		m_mcu(*this, "mcu"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_custom_cpu_ram;
	required_shared_ptr<uint8_t> m_blitter;

	std::unique_ptr<uint8_t[]>  m_main_bitmap;
	std::unique_ptr<uint8_t[]>  m_converted_gfx;

	/* video-related */
	uint8_t    m_video_control;
	uint8_t    m_flip_screen;

	/* MCU */
	uint8_t    m_mcu_port_o;
	uint8_t    m_mcu_port_p;
	uint8_t    m_mcu_port_r[4];
	uint8_t mcu_port_r_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mcu_port_r_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mcu_portk_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mcu_port_o_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mcu_port_p_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void arabian_blitter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void arabian_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ay8910_porta_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ay8910_portb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_arabian(palette_device &palette);
	uint32_t screen_update_arabian(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void blit_area( uint8_t plane, uint16_t src, uint8_t x, uint8_t y, uint8_t sx, uint8_t sy );
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_mcu;
	required_device<palette_device> m_palette;
};
