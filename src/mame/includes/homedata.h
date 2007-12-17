/*----------- defined in video/homedata.c -----------*/

extern UINT8 *homedata_vreg;
extern int homedata_visible_page;
extern int homedata_priority;
extern UINT8 reikaids_which;

WRITE8_HANDLER( mrokumei_videoram_w );
WRITE8_HANDLER( reikaids_videoram_w );
WRITE8_HANDLER( pteacher_videoram_w );
WRITE8_HANDLER( reikaids_gfx_bank_w );
WRITE8_HANDLER( pteacher_gfx_bank_w );
WRITE8_HANDLER( homedata_blitter_param_w );
WRITE8_HANDLER( mrokumei_blitter_bank_w );
WRITE8_HANDLER( reikaids_blitter_bank_w );
WRITE8_HANDLER( pteacher_blitter_bank_w );
WRITE8_HANDLER( mrokumei_blitter_start_w );
WRITE8_HANDLER( reikaids_blitter_start_w );
WRITE8_HANDLER( pteacher_blitter_start_w );

PALETTE_INIT( mrokumei );
PALETTE_INIT( reikaids );
PALETTE_INIT( pteacher );

VIDEO_START( mrokumei );
VIDEO_START( reikaids );
VIDEO_START( pteacher );
VIDEO_START( lemnangl );
VIDEO_UPDATE( mrokumei );
VIDEO_UPDATE( reikaids );
VIDEO_UPDATE( pteacher );
VIDEO_EOF( homedata );
