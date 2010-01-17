typedef struct _pass_state pass_state;
struct _pass_state
{
	tilemap_t *bg_tilemap;
	tilemap_t *fg_tilemap;

	UINT16 *bg_videoram;
	UINT16 *fg_videoram;
};


/*----------- defined in video/pass.c -----------*/

VIDEO_START( pass );
VIDEO_UPDATE( pass );
WRITE16_HANDLER( pass_fg_videoram_w );
WRITE16_HANDLER( pass_bg_videoram_w );
