/***************************************************************************

    Game Driver for Video System Mahjong series and Pipe Dream.

    Driver by Takahiro Nogi <nogi@kt.rim.or.jp> 2001/02/04 -
    and Bryan McPhail, Nicola Salmoria, Aaron Giles

***************************************************************************/

/*----------- defined in video/fromance.c -----------*/

VIDEO_START( fromance );
VIDEO_START( nekkyoku );
VIDEO_START( pipedrm );
VIDEO_START( hatris );
VIDEO_UPDATE( fromance );
VIDEO_UPDATE( pipedrm );

WRITE8_HANDLER( fromance_crtc_data_w );
WRITE8_HANDLER( fromance_crtc_register_w );

WRITE8_HANDLER( fromance_gfxreg_w );

WRITE8_HANDLER( fromance_scroll_w );

READ8_HANDLER( fromance_paletteram_r );
WRITE8_HANDLER( fromance_paletteram_w );

READ8_HANDLER( fromance_videoram_r );
WRITE8_HANDLER( fromance_videoram_w );
