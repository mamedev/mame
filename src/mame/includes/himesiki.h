/*************************************************************************

    Himeshikibu

*************************************************************************/

typedef struct _himesiki_state himesiki_state;
struct _himesiki_state
{
	/* memory pointers */
	UINT8 *    bg_ram;
	UINT8 *    spriteram;
//  UINT8 *    paletteram;  // currently this uses generic palette handling

	/* video-related */
	tilemap_t    *bg_tilemap;
	int 	     scrollx[2];
	int        flipscreen;

	/* devices */
	const device_config *subcpu;
};


/*----------- defined in video/himesiki.c -----------*/

VIDEO_START( himesiki );
VIDEO_UPDATE( himesiki );

WRITE8_HANDLER( himesiki_bg_ram_w );
WRITE8_HANDLER( himesiki_scrollx_w );
WRITE8_HANDLER( himesiki_flip_w );
