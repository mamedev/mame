/*************************************************************************

    Driver for Midway Zeus games

**************************************************************************/

#define MIDZEUS_VIDEO_CLOCK		XTAL_66_6667MHz

class midzeus_state : public driver_device
{
public:
	midzeus_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_nvram(*this, "nvram") { }

	required_shared_ptr<UINT32>	m_nvram;
	DECLARE_WRITE32_MEMBER(cmos_w);
	DECLARE_READ32_MEMBER(cmos_r);
	DECLARE_WRITE32_MEMBER(cmos_protect_w);
	DECLARE_READ32_MEMBER(zpram_r);
	DECLARE_WRITE32_MEMBER(zpram_w);
	DECLARE_READ32_MEMBER(bitlatches_r);
	DECLARE_WRITE32_MEMBER(bitlatches_w);
	DECLARE_READ32_MEMBER(crusnexo_leds_r);
	DECLARE_WRITE32_MEMBER(crusnexo_leds_w);
	DECLARE_READ32_MEMBER(linkram_r);
	DECLARE_WRITE32_MEMBER(linkram_w);
	DECLARE_READ32_MEMBER(tms32031_control_r);
	DECLARE_WRITE32_MEMBER(tms32031_control_w);
	DECLARE_WRITE32_MEMBER(keypad_select_w);
	DECLARE_READ32_MEMBER(analog_r);
	DECLARE_WRITE32_MEMBER(analog_w);
	DECLARE_WRITE32_MEMBER(invasn_gun_w);
	DECLARE_READ32_MEMBER(invasn_gun_r);
	DECLARE_READ8_MEMBER(PIC16C5X_T0_clk_r);
	DECLARE_READ32_MEMBER(zeus_r);
	DECLARE_WRITE32_MEMBER(zeus_w);
};


/*----------- defined in video/midzeus.c -----------*/

extern UINT32 *zeusbase;

VIDEO_START( midzeus );
SCREEN_UPDATE_IND16( midzeus );


/*----------- defined in video/midzeus2.c -----------*/

VIDEO_START( midzeus2 );
SCREEN_UPDATE_RGB32( midzeus2 );

READ32_HANDLER( zeus2_r );
WRITE32_HANDLER( zeus2_w );

