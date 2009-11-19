/***************************************************************************

    Battle Cross

***************************************************************************/

typedef struct _battlex_state battlex_state;
struct _battlex_state
{
	/* memory pointers */
	UINT8 * videoram;
	UINT8 * spriteram;

	/* video-related */
	tilemap *bg_tilemap;
	int     scroll_lsb, scroll_msb;
};


/*----------- defined in video/battlex.c -----------*/

extern WRITE8_HANDLER( battlex_palette_w );
extern WRITE8_HANDLER( battlex_videoram_w );
extern WRITE8_HANDLER( battlex_scroll_x_lsb_w );
extern WRITE8_HANDLER( battlex_scroll_x_msb_w );
extern WRITE8_HANDLER( battlex_flipscreen_w );

extern PALETTE_INIT( battlex );
extern VIDEO_START( battlex );
extern VIDEO_UPDATE( battlex );
