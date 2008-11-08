/*----------- defined in video/nbmj8891.c -----------*/

VIDEO_UPDATE( nbmj8891 );
VIDEO_START( nbmj8891_1layer );
VIDEO_START( nbmj8891_2layer );

READ8_HANDLER( nbmj8891_palette_type1_r );
WRITE8_HANDLER( nbmj8891_palette_type1_w );
READ8_HANDLER( nbmj8891_palette_type2_r );
WRITE8_HANDLER( nbmj8891_palette_type2_w );
READ8_HANDLER( nbmj8891_palette_type3_r );
WRITE8_HANDLER( nbmj8891_palette_type3_w );
WRITE8_HANDLER( nbmj8891_blitter_w );
WRITE8_HANDLER( nbmj8891_scrolly_w );
WRITE8_HANDLER( nbmj8891_vramsel_w );
WRITE8_HANDLER( nbmj8891_romsel_w );
WRITE8_HANDLER( nbmj8891_clutsel_w );
READ8_HANDLER( nbmj8891_clut_r );
WRITE8_HANDLER( nbmj8891_clut_w );
WRITE8_HANDLER( nbmj8891_taiwanmb_blitter_w );
WRITE8_HANDLER( nbmj8891_taiwanmb_gfxflag_w );
WRITE8_HANDLER( nbmj8891_taiwanmb_gfxdraw_w );
WRITE8_HANDLER( nbmj8891_taiwanmb_mcu_w );
