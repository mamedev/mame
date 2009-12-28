/*************************************************************************

    Circus Charlie

*************************************************************************/

typedef struct _circusc_state circusc_state;
struct _circusc_state
{
	/* memory pointers */
	UINT8 *        videoram;
	UINT8 *        colorram;
	UINT8 *        spriteram;
	UINT8 *        spriteram_2;
	UINT8 *        spritebank;
	UINT8 *        scroll;
	size_t         spriteram_size;

	/* video-related */
	tilemap_t        *bg_tilemap;

	/* sound-related */
	UINT8          sn_latch;

	/* devices */
	const device_config *audiocpu;
	const device_config *sn1;
	const device_config *sn2;
	const device_config *dac;
	const device_config *discrete;
};


/*----------- defined in video/circusc.c -----------*/

WRITE8_HANDLER( circusc_videoram_w );
WRITE8_HANDLER( circusc_colorram_w );

VIDEO_START( circusc );
WRITE8_HANDLER( circusc_flipscreen_w );
PALETTE_INIT( circusc );
VIDEO_UPDATE( circusc );
