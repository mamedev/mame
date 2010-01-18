/*************************************************************************

    KO Punch

*************************************************************************/

typedef struct _kopunch_state kopunch_state;
struct _kopunch_state
{
	/* memory pointers */
	UINT8 *    videoram;
	UINT8 *    videoram2;
	UINT8 *    colorram;
	UINT8 *    spriteram;
	size_t     spriteram_size;

	/* video-related */
	tilemap_t    *bg_tilemap, *fg_tilemap;
	int        gfxbank;

	/* devices */
	running_device *maincpu;
};

/*----------- defined in video/kopunch.c -----------*/

WRITE8_HANDLER( kopunch_videoram_w );
WRITE8_HANDLER( kopunch_videoram2_w );
WRITE8_HANDLER( kopunch_scroll_x_w );
WRITE8_HANDLER( kopunch_scroll_y_w );
WRITE8_HANDLER( kopunch_gfxbank_w );

PALETTE_INIT( kopunch );
VIDEO_START( kopunch );
VIDEO_UPDATE( kopunch );
