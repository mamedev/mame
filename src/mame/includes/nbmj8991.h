/*----------- defined in video/nbmj8991.c -----------*/

VIDEO_UPDATE( nbmj8991_type1 );
VIDEO_UPDATE( nbmj8991_type2 );
VIDEO_START( nbmj8991 );

WRITE8_HANDLER( nbmj8991_palette_type1_w );
WRITE8_HANDLER( nbmj8991_palette_type2_w );
WRITE8_HANDLER( nbmj8991_palette_type3_w );
WRITE8_HANDLER( nbmj8991_blitter_w );
READ8_HANDLER( nbmj8991_clut_r );
WRITE8_HANDLER( nbmj8991_clut_w );
