/*************************************************************************

    Knuckle Joe

*************************************************************************/

typedef struct _kncljoe_state kncljoe_state;
struct _kncljoe_state
{
	/* memory pointers */
	UINT8 *    videoram;
	UINT8 *    spriteram;
	UINT8 *    scrollregs;
	size_t     spriteram_size;

	/* video-related */
	tilemap    *bg_tilemap;
	int        tile_bank, sprite_bank;
	int        flipscreen;

	/* misc */
	UINT8      port1, port2;

	/* devices */
	const device_config *soundcpu;
};



/*----------- defined in video/kncljoe.c -----------*/

WRITE8_HANDLER(kncljoe_videoram_w);
WRITE8_HANDLER(kncljoe_control_w);
WRITE8_HANDLER(kncljoe_scroll_w);

PALETTE_INIT( kncljoe );
VIDEO_START( kncljoe );
VIDEO_UPDATE( kncljoe );
