/*----------- defined in video/nbmj8900.c -----------*/

VIDEO_UPDATE( nbmj8900 );
VIDEO_START( nbmj8900_2layer );

READ8_HANDLER( nbmj8900_palette_type1_r );
WRITE8_HANDLER( nbmj8900_palette_type1_w );
WRITE8_HANDLER( nbmj8900_blitter_w );
WRITE8_HANDLER( nbmj8900_scrolly_w );
WRITE8_HANDLER( nbmj8900_vramsel_w );
WRITE8_HANDLER( nbmj8900_romsel_w );
WRITE8_HANDLER( nbmj8900_clutsel_w );
READ8_HANDLER( nbmj8900_clut_r );
WRITE8_HANDLER( nbmj8900_clut_w );
