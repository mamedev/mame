/***************************************************************************

    Blue Print

***************************************************************************/

typedef struct _blueprnt_state blueprnt_state;
struct _blueprnt_state
{
	/* memory pointers */
	UINT8 * videoram;
	UINT8 * colorram;
	UINT8 * spriteram;
	UINT8 * scrollram;
	size_t  spriteram_size;

	/* video-related */
	tilemap_t *bg_tilemap;
	int     gfx_bank;

	/* misc */
	int     dipsw;

	/* devices */
	const device_config *audiocpu;
};


/*----------- defined in video/blueprnt.c -----------*/

WRITE8_HANDLER( blueprnt_videoram_w );
WRITE8_HANDLER( blueprnt_colorram_w );
WRITE8_HANDLER( blueprnt_flipscreen_w );

PALETTE_INIT( blueprnt );
VIDEO_START( blueprnt );
VIDEO_UPDATE( blueprnt );
