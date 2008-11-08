/*----------- defined in video/nbmj8688.c -----------*/

PALETTE_INIT( mbmj8688_8bit );
PALETTE_INIT( mbmj8688_12bit );
PALETTE_INIT( mbmj8688_16bit );
VIDEO_UPDATE( mbmj8688 );
VIDEO_UPDATE( mbmj8688_LCD );
VIDEO_START( mbmj8688_8bit );
VIDEO_START( mbmj8688_hybrid_12bit );
VIDEO_START( mbmj8688_pure_12bit );
VIDEO_START( mbmj8688_hybrid_16bit );
VIDEO_START( mbmj8688_pure_16bit );
VIDEO_START( mbmj8688_pure_16bit_LCD );

WRITE8_HANDLER( nbmj8688_clut_w );
WRITE8_HANDLER( nbmj8688_blitter_w );
WRITE8_HANDLER( mjsikaku_gfxflag2_w );
WRITE8_HANDLER( mjsikaku_scrolly_w );
WRITE8_HANDLER( mjsikaku_romsel_w );
WRITE8_HANDLER( secolove_romsel_w );
WRITE8_HANDLER( seiha_romsel_w );
WRITE8_HANDLER( crystalg_romsel_w );

WRITE8_HANDLER( nbmj8688_HD61830B_0_instr_w );
WRITE8_HANDLER( nbmj8688_HD61830B_0_data_w );
WRITE8_HANDLER( nbmj8688_HD61830B_1_instr_w );
WRITE8_HANDLER( nbmj8688_HD61830B_1_data_w );
WRITE8_HANDLER( nbmj8688_HD61830B_both_instr_w );
WRITE8_HANDLER( nbmj8688_HD61830B_both_data_w );
