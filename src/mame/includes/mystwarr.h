/*----------- defined in video/mystwarr.c -----------*/

VIDEO_START( gaiapols );
VIDEO_START( dadandrn );
VIDEO_START( viostorm );
VIDEO_START( metamrph );
VIDEO_START( martchmp );
VIDEO_START( mystwarr );
VIDEO_UPDATE( dadandrn );
VIDEO_UPDATE( mystwarr );
VIDEO_UPDATE( metamrph );
VIDEO_UPDATE( martchmp );

WRITE16_HANDLER( ddd_053936_enable_w );
WRITE16_HANDLER( ddd_053936_clip_w );
READ16_HANDLER( gai_053936_tilerom_0_r );
READ16_HANDLER( ddd_053936_tilerom_0_r );
READ16_HANDLER( ddd_053936_tilerom_1_r );
READ16_HANDLER( gai_053936_tilerom_2_r );
READ16_HANDLER( ddd_053936_tilerom_2_r );
