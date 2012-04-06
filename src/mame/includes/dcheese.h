/***************************************************************************

    HAR MadMax hardware

**************************************************************************/


class dcheese_state : public driver_device
{
public:
	dcheese_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* video-related */
	UINT16   m_blitter_color[2];
	UINT16   m_blitter_xparam[16];
	UINT16   m_blitter_yparam[16];
	UINT16   m_blitter_vidparam[32];

	bitmap_ind16 *m_dstbitmap;
	emu_timer *m_blitter_timer;

	/* misc */
	UINT8    m_irq_state[5];
	UINT8    m_soundlatch_full;
	UINT8    m_sound_control;
	UINT8    m_sound_msb_latch;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_bsmt;
	DECLARE_WRITE16_MEMBER(eeprom_control_w);
	DECLARE_WRITE16_MEMBER(sound_command_w);
	DECLARE_READ8_MEMBER(sound_command_r);
	DECLARE_READ8_MEMBER(sound_status_r);
	DECLARE_WRITE8_MEMBER(sound_control_w);
	DECLARE_WRITE8_MEMBER(bsmt_data_w);
	DECLARE_WRITE16_MEMBER(madmax_blitter_color_w);
	DECLARE_WRITE16_MEMBER(madmax_blitter_xparam_w);
	DECLARE_WRITE16_MEMBER(madmax_blitter_yparam_w);
	DECLARE_WRITE16_MEMBER(madmax_blitter_vidparam_w);
	DECLARE_WRITE16_MEMBER(madmax_blitter_unknown_w);
	DECLARE_READ16_MEMBER(madmax_blitter_vidparam_r);
};


/*----------- defined in drivers/dcheese.c -----------*/

void dcheese_signal_irq(running_machine &machine, int which);


/*----------- defined in video/dcheese.c -----------*/

PALETTE_INIT( dcheese );
VIDEO_START( dcheese );
SCREEN_UPDATE_IND16( dcheese );


