/*************************************************************************

    Gumbo - Miss Bingo - Miss Puzzle

*************************************************************************/

typedef struct _gumbo_state gumbo_state;
struct _gumbo_state
{
	/* memory pointers */
	UINT16 *    bg_videoram;
	UINT16 *    fg_videoram;
//  UINT16 *    paletteram; // currently this uses generic palette handling

	/* video-related */
	tilemap    *bg_tilemap;
	tilemap    *fg_tilemap;
};


/*----------- defined in video/gumbo.c -----------*/

WRITE16_HANDLER( gumbo_bg_videoram_w );
WRITE16_HANDLER( gumbo_fg_videoram_w );

VIDEO_START( gumbo );
VIDEO_UPDATE( gumbo );
