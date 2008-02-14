/*************************************************************************

    Atari Cloak & Dagger hardware

*************************************************************************/

/*----------- defined in video/cloak.c -----------*/

WRITE8_HANDLER( cloak_paletteram_w );
READ8_HANDLER( graph_processor_r );
WRITE8_HANDLER( graph_processor_w );
WRITE8_HANDLER( cloak_clearbmp_w );

VIDEO_START( cloak );
VIDEO_UPDATE( cloak );
