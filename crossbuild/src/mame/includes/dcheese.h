/***************************************************************************

    HAR MadMax hardware

**************************************************************************/


/*----------- defined in drivers/dcheese.c -----------*/

void dcheese_signal_irq(int which);


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
