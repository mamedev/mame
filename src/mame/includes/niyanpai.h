/*----------- defined in video/niyanpai.c -----------*/

VIDEO_UPDATE( niyanpai );
VIDEO_START( niyanpai );

READ16_HANDLER( niyanpai_palette_r );
WRITE16_HANDLER( niyanpai_palette_w );

READ16_HANDLER( niyanpai_blitter_0_r );
READ16_HANDLER( niyanpai_blitter_1_r );
READ16_HANDLER( niyanpai_blitter_2_r );
WRITE16_HANDLER( niyanpai_blitter_0_w );
WRITE16_HANDLER( niyanpai_blitter_1_w );
WRITE16_HANDLER( niyanpai_blitter_2_w );
WRITE16_HANDLER( niyanpai_clut_0_w );
WRITE16_HANDLER( niyanpai_clut_1_w );
WRITE16_HANDLER( niyanpai_clut_2_w );
WRITE16_HANDLER( niyanpai_clutsel_0_w );
WRITE16_HANDLER( niyanpai_clutsel_1_w );
WRITE16_HANDLER( niyanpai_clutsel_2_w );
