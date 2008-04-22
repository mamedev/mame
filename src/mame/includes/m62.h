/*************************************************************************

    Irem M62 hardware

*************************************************************************/

/*----------- defined in video/m62.c -----------*/

PALETTE_INIT( m62 );
PALETTE_INIT( lotlot );
PALETTE_INIT( battroad );
PALETTE_INIT( spelunk2 );

WRITE8_HANDLER( m62_tileram_w );
WRITE8_HANDLER( m62_textram_w );
WRITE8_HANDLER( m62_flipscreen_w );
WRITE8_HANDLER( m62_hscroll_low_w );
WRITE8_HANDLER( m62_hscroll_high_w );
WRITE8_HANDLER( m62_vscroll_low_w );
WRITE8_HANDLER( m62_vscroll_high_w );
WRITE8_HANDLER( ldrun3_topbottom_mask_w );
extern UINT8 *m62_tileram;
extern UINT8 *m62_textram;

VIDEO_START( kungfum );
VIDEO_UPDATE( kungfum );
WRITE8_HANDLER( kungfum_tileram_w );

VIDEO_START( ldrun );
VIDEO_UPDATE( ldrun );

VIDEO_START( ldrun2 );

VIDEO_UPDATE( ldrun3 );

VIDEO_START( battroad );
VIDEO_UPDATE( battroad );

VIDEO_START( ldrun4 );
VIDEO_UPDATE( ldrun4 );

VIDEO_START( lotlot );
VIDEO_UPDATE( lotlot );

WRITE8_HANDLER( kidniki_text_vscroll_low_w );
WRITE8_HANDLER( kidniki_text_vscroll_high_w );
WRITE8_HANDLER( kidniki_background_bank_w );
VIDEO_START( kidniki );
VIDEO_UPDATE( kidniki );

WRITE8_HANDLER( spelunkr_palbank_w );
VIDEO_START( spelunkr );
VIDEO_UPDATE( spelunkr );

WRITE8_HANDLER( spelunk2_gfxport_w );
VIDEO_START( spelunk2 );
VIDEO_UPDATE( spelunk2 );

VIDEO_START( youjyudn );
VIDEO_UPDATE( youjyudn );

VIDEO_START( horizon );
VIDEO_UPDATE( horizon );
WRITE8_HANDLER( horizon_scrollram_w );
extern UINT8 *horizon_scrollram;
