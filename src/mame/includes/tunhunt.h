typedef struct _tunhunt_state tunhunt_state;
struct _tunhunt_state
{
	UINT8 control;
	UINT8 *workram;
	UINT8 *spriteram;
	UINT8 *videoram;
	tilemap_t *fg_tilemap;
};


/*----------- defined in video/tunhunt.c -----------*/

WRITE8_HANDLER( tunhunt_videoram_w );

PALETTE_INIT( tunhunt );
VIDEO_START( tunhunt );
VIDEO_UPDATE( tunhunt );
