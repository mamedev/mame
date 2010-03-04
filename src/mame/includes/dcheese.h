/***************************************************************************

    HAR MadMax hardware

**************************************************************************/


class dcheese_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, dcheese_state(machine)); }

	dcheese_state(running_machine &machine) { }

	/* video-related */
	UINT16   blitter_color[2];
	UINT16   blitter_xparam[16];
	UINT16   blitter_yparam[16];
	UINT16   blitter_vidparam[32];

	bitmap_t *dstbitmap;
	emu_timer *blitter_timer;

	/* misc */
	UINT8    irq_state[5];
	UINT8    soundlatch_full;
	UINT8    sound_control;
	UINT8    sound_msb_latch;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
	running_device *bsmt;
};


/*----------- defined in drivers/dcheese.c -----------*/

void dcheese_signal_irq(running_machine *machine, int which);


/*----------- defined in video/dcheese.c -----------*/

PALETTE_INIT( dcheese );
VIDEO_START( dcheese );
VIDEO_UPDATE( dcheese );

WRITE16_HANDLER( madmax_blitter_color_w );
WRITE16_HANDLER( madmax_blitter_xparam_w );
WRITE16_HANDLER( madmax_blitter_yparam_w );
WRITE16_HANDLER( madmax_blitter_vidparam_w );
WRITE16_HANDLER( madmax_blitter_unknown_w );

READ16_HANDLER( madmax_blitter_vidparam_r );
