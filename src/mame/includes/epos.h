// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/*************************************************************************

    Epos games

**************************************************************************/

class epos_state : public driver_device
{
public:
	epos_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;

	/* video-related */
	UINT8    m_palette;

	/* misc */
	int      m_counter;
	DECLARE_WRITE8_MEMBER(dealer_decrypt_rom);
	DECLARE_WRITE8_MEMBER(epos_port_1_w);
	DECLARE_WRITE8_MEMBER(write_prtc);
	DECLARE_DRIVER_INIT(dealer);
	virtual void machine_reset() override;
	DECLARE_MACHINE_START(epos);
	DECLARE_MACHINE_START(dealer);
	UINT32 screen_update_epos(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void get_pens( pen_t *pens );
	required_device<cpu_device> m_maincpu;
};
