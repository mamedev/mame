// license:BSD-3-Clause
// copyright-holders:Dan Boris
/***************************************************************************

    Sun Electronics Arabian hardware

    driver by Dan Boris

***************************************************************************/

class arabian_state : public driver_device
{
public:
	arabian_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_custom_cpu_ram(*this, "custom_cpu_ram"),
		m_blitter(*this, "blitter"),
		m_maincpu(*this, "maincpu"),
		m_mcu(*this, "mcu"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_custom_cpu_ram;
	required_shared_ptr<UINT8> m_blitter;

	std::unique_ptr<UINT8[]>  m_main_bitmap;
	std::unique_ptr<UINT8[]>  m_converted_gfx;

	/* video-related */
	UINT8    m_video_control;
	UINT8    m_flip_screen;

	/* MCU */
	UINT8    m_mcu_port_o;
	UINT8    m_mcu_port_p;
	UINT8    m_mcu_port_r[4];
	DECLARE_READ8_MEMBER(mcu_port_r_r);
	DECLARE_WRITE8_MEMBER(mcu_port_r_w);
	DECLARE_READ8_MEMBER(mcu_portk_r);
	DECLARE_WRITE8_MEMBER(mcu_port_o_w);
	DECLARE_WRITE8_MEMBER(mcu_port_p_w);
	DECLARE_WRITE8_MEMBER(arabian_blitter_w);
	DECLARE_WRITE8_MEMBER(arabian_videoram_w);
	DECLARE_WRITE8_MEMBER(ay8910_porta_w);
	DECLARE_WRITE8_MEMBER(ay8910_portb_w);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(arabian);
	UINT32 screen_update_arabian(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void blit_area( UINT8 plane, UINT16 src, UINT8 x, UINT8 y, UINT8 sx, UINT8 sy );
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_mcu;
	required_device<palette_device> m_palette;
};
