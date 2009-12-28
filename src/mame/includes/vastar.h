typedef struct _vastar_state vastar_state;
struct _vastar_state
{
	UINT8 *spriteram1;
	UINT8 *spriteram2;
	UINT8 *spriteram3;

	UINT8 *bg1videoram;
	UINT8 *bg2videoram;
	UINT8 *fgvideoram;
	UINT8 *bg1_scroll;
	UINT8 *bg2_scroll;
	UINT8 *sprite_priority;

	tilemap_t *fg_tilemap;
	tilemap_t *bg1_tilemap;
	tilemap_t *bg2_tilemap;

	UINT8 *sharedram;
};


/*----------- defined in video/vastar.c -----------*/

WRITE8_HANDLER( vastar_bg1videoram_w );
WRITE8_HANDLER( vastar_bg2videoram_w );
WRITE8_HANDLER( vastar_fgvideoram_w );
READ8_HANDLER( vastar_bg1videoram_r );
READ8_HANDLER( vastar_bg2videoram_r );

VIDEO_START( vastar );
VIDEO_UPDATE( vastar );
