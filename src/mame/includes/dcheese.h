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

	bitmap_t *m_dstbitmap;
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
};


/*----------- defined in drivers/dcheese.c -----------*/

void dcheese_signal_irq(running_machine &machine, int which);


/*----------- defined in video/dcheese.c -----------*/

PALETTE_INIT( dcheese );
VIDEO_START( dcheese );
SCREEN_UPDATE( dcheese );

WRITE16_HANDLER( madmax_blitter_color_w );
WRITE16_HANDLER( madmax_blitter_xparam_w );
WRITE16_HANDLER( madmax_blitter_yparam_w );
WRITE16_HANDLER( madmax_blitter_vidparam_w );
WRITE16_HANDLER( madmax_blitter_unknown_w );

READ16_HANDLER( madmax_blitter_vidparam_r );
