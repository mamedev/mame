/*************************************************************************

    Irem Z80 hardware - M52, M57, M58 and M62 boards

*************************************************************************/

/* These share sound hardware (in audio/irem.h) and hence driver data */

typedef struct _irem_z80_state irem_z80_state;
struct _irem_z80_state
{
	/* memory pointers */
	UINT8 *              videoram;	// m52, m57, m58
	UINT8 *              spriteram;	// m52, m57, m58, m62
	size_t               spriteram_size;

	UINT8 *              colorram;	// m52
	UINT8 *              m62_tileram;	// m62
	UINT8 *              m62_textram;	// m62

	UINT8 *              scrollram;	// m57, m62 (horizon only)

	/* sound-related */
	UINT8                port1, port2;

	/* video-related */
	tilemap*             bg_tilemap;	// m52, m57, m58, m62
	tilemap*             fg_tilemap;	// m62
	int                  flipscreen;	// m57, m62

	/* driver specific (video-related) */
	/* travrusa */
	int                  scrollx[2];
	/* M52 */
	UINT8                bg1xpos, bg1ypos;
	UINT8                bg2xpos, bg2ypos;
	UINT8                bgcontrol;
	/* M58 */
	UINT8                *yard_scroll_x_low;
	UINT8                *yard_scroll_x_high;
	UINT8                *yard_scroll_y_low;
	UINT8                *yard_score_panel_disabled;
	bitmap_t             *scroll_panel_bitmap;
	/* M62 */
	const UINT8          *sprite_height_prom;
	INT32                m62_background_hscroll;
	INT32                m62_background_vscroll;
	UINT8                kidniki_background_bank;
	INT32                kidniki_text_vscroll;
	int                  ldrun3_topbottom_mask;
	INT32                spelunkr_palbank;


	/* misc */
	int                 ldrun2_bankswap;	//ldrun2
	int                 bankcontrol[2]; 	//ldrun2

	/* sound devices */
	const device_config *ay1;
	const device_config *ay2;
	const device_config *adpcm1;
	const device_config *adpcm2;
};


/*----------- defined in audio/irem.c -----------*/

MACHINE_DRIVER_EXTERN( m52_sound_c_audio );
MACHINE_DRIVER_EXTERN( m52_large_audio );
MACHINE_DRIVER_EXTERN( m62_audio );

WRITE8_HANDLER( irem_sound_cmd_w );



/* Video hardware is different otoh  */

/*----------- defined in video/m52.c -----------*/

READ8_HANDLER( m52_protection_r );
WRITE8_HANDLER( m52_scroll_w );
WRITE8_HANDLER( m52_bg1xpos_w );
WRITE8_HANDLER( m52_bg1ypos_w );
WRITE8_HANDLER( m52_bg2xpos_w );
WRITE8_HANDLER( m52_bg2ypos_w );
WRITE8_HANDLER( m52_bgcontrol_w );
WRITE8_HANDLER( m52_flipscreen_w );
WRITE8_HANDLER( alpha1v_flipscreen_w );
WRITE8_HANDLER( m52_videoram_w );
WRITE8_HANDLER( m52_colorram_w );

PALETTE_INIT( m52 );
VIDEO_START( m52 );
VIDEO_UPDATE( m52 );

/*----------- defined in video/m57.c -----------*/

WRITE8_HANDLER( m57_videoram_w );
WRITE8_HANDLER( m57_flipscreen_w );

PALETTE_INIT( m57 );
VIDEO_START( m57 );
VIDEO_UPDATE( m57 );


/*----------- defined in video/m58.c -----------*/

WRITE8_HANDLER( yard_videoram_w );
WRITE8_HANDLER( yard_scroll_panel_w );
WRITE8_HANDLER( yard_flipscreen_w );

PALETTE_INIT( yard );
VIDEO_START( yard );
VIDEO_UPDATE( yard );


/*----------- defined in video/travrusa.c -----------*/

WRITE8_HANDLER( travrusa_videoram_w );
WRITE8_HANDLER( travrusa_scroll_x_low_w );
WRITE8_HANDLER( travrusa_scroll_x_high_w );
WRITE8_HANDLER( travrusa_flipscreen_w );

PALETTE_INIT( travrusa );
PALETTE_INIT( shtrider );
VIDEO_START( travrusa );
VIDEO_UPDATE( travrusa );


/*----------- defined in video/m62.c -----------*/

WRITE8_HANDLER( m62_tileram_w );
WRITE8_HANDLER( m62_textram_w );
WRITE8_HANDLER( m62_flipscreen_w );
WRITE8_HANDLER( m62_hscroll_low_w );
WRITE8_HANDLER( m62_hscroll_high_w );
WRITE8_HANDLER( m62_vscroll_low_w );
WRITE8_HANDLER( m62_vscroll_high_w );

WRITE8_HANDLER( horizon_scrollram_w );
WRITE8_HANDLER( kidniki_text_vscroll_low_w );
WRITE8_HANDLER( kidniki_text_vscroll_high_w );
WRITE8_HANDLER( kidniki_background_bank_w );
WRITE8_HANDLER( kungfum_tileram_w );
WRITE8_HANDLER( ldrun3_topbottom_mask_w );
WRITE8_HANDLER( spelunkr_palbank_w );
WRITE8_HANDLER( spelunk2_gfxport_w );

PALETTE_INIT( m62 );
PALETTE_INIT( lotlot );
PALETTE_INIT( battroad );
PALETTE_INIT( spelunk2 );

VIDEO_START( battroad );
VIDEO_START( horizon );
VIDEO_START( kidniki );
VIDEO_START( kungfum );
VIDEO_START( ldrun );
VIDEO_START( ldrun2 );
VIDEO_START( ldrun4 );
VIDEO_START( lotlot );
VIDEO_START( spelunkr );
VIDEO_START( spelunk2 );
VIDEO_START( youjyudn );

VIDEO_UPDATE( battroad );
VIDEO_UPDATE( horizon );
VIDEO_UPDATE( kidniki );
VIDEO_UPDATE( kungfum );
VIDEO_UPDATE( ldrun );
VIDEO_UPDATE( ldrun3 );
VIDEO_UPDATE( ldrun4 );
VIDEO_UPDATE( lotlot );
VIDEO_UPDATE( spelunkr );
VIDEO_UPDATE( spelunk2 );
VIDEO_UPDATE( youjyudn );
