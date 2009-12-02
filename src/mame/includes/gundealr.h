/*************************************************************************

    Gun Dealer

*************************************************************************/

typedef struct _gundealr_state gundealr_state;
struct _gundealr_state
{
	/* memory pointers */
	UINT8 *    bg_videoram;
	UINT8 *    fg_videoram;
	UINT8 *    rambase;
	UINT8 *    paletteram;

	/* video-related */
	tilemap    *bg_tilemap;
	tilemap    *fg_tilemap;
	int        flipscreen;
	UINT8      scroll[4];

	/* misc */
	int        input_ports_hack;
};



/*----------- defined in video/gundealr.c -----------*/

WRITE8_HANDLER( gundealr_paletteram_w );
WRITE8_HANDLER( gundealr_bg_videoram_w );
WRITE8_HANDLER( gundealr_fg_videoram_w );
WRITE8_HANDLER( gundealr_fg_scroll_w );
WRITE8_HANDLER( yamyam_fg_scroll_w );
WRITE8_HANDLER( gundealr_flipscreen_w );

VIDEO_UPDATE( gundealr );
VIDEO_START( gundealr );
