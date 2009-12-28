/*************************************************************************

    Gun.Smoke

*************************************************************************/

typedef struct _gunsmoke_state gunsmoke_state;
struct _gunsmoke_state
{
	/* memory pointers */
	UINT8 *    videoram;
	UINT8 *    colorram;
	UINT8 *    spriteram;
	UINT8 *    scrollx;
	UINT8 *    scrolly;
	size_t     spriteram_size;

	/* video-related */
	tilemap_t    *bg_tilemap, *fg_tilemap;
	UINT8      chon, objon, bgon;
	UINT8      sprite3bank;
};


/*----------- defined in video/gunsmoke.c -----------*/

WRITE8_HANDLER( gunsmoke_c804_w );
WRITE8_HANDLER( gunsmoke_d806_w );
WRITE8_HANDLER( gunsmoke_videoram_w );
WRITE8_HANDLER( gunsmoke_colorram_w );

PALETTE_INIT( gunsmoke );
VIDEO_START( gunsmoke );
VIDEO_UPDATE( gunsmoke );

