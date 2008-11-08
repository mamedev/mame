/*----------- defined in video/nbmj9195.c -----------*/

VIDEO_UPDATE( nbmj9195 );
VIDEO_START( nbmj9195_1layer );
VIDEO_START( nbmj9195_2layer );
VIDEO_START( nbmj9195_nb22090 );

READ8_HANDLER( nbmj9195_palette_r );
WRITE8_HANDLER( nbmj9195_palette_w );
READ8_HANDLER( nbmj9195_nb22090_palette_r );
WRITE8_HANDLER( nbmj9195_nb22090_palette_w );

READ8_HANDLER( nbmj9195_blitter_0_r );
READ8_HANDLER( nbmj9195_blitter_1_r );
WRITE8_HANDLER( nbmj9195_blitter_0_w );
WRITE8_HANDLER( nbmj9195_blitter_1_w );
WRITE8_HANDLER( nbmj9195_clut_0_w );
WRITE8_HANDLER( nbmj9195_clut_1_w );

void nbmj9195_clutsel_w(int data);
void nbmj9195_gfxflag2_w(int data);
