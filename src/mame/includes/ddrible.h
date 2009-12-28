/***************************************************************************

    Double Dribble

***************************************************************************/

typedef struct _ddrible_state ddrible_state;
struct _ddrible_state
{
	/* memory pointers */
	UINT8 *     sharedram;
	UINT8 *     snd_sharedram;
	UINT8 *     spriteram_1;
	UINT8 *     spriteram_2;
	UINT8 *     bg_videoram;
	UINT8 *     fg_videoram;
	UINT8 *     paletteram;

	/* video-related */
	tilemap_t     *fg_tilemap,*bg_tilemap;
	int         vregs[2][5];
	int         charbank[2];

	/* misc */
	int         int_enable_0, int_enable_1;

	/* devices */
	const device_config *filter1;
	const device_config *filter2;
	const device_config *filter3;
};

/*----------- defined in video/ddrible.c -----------*/

WRITE8_HANDLER( ddrible_fg_videoram_w );
WRITE8_HANDLER( ddrible_bg_videoram_w );
WRITE8_HANDLER( K005885_0_w );
WRITE8_HANDLER( K005885_1_w );

PALETTE_INIT( ddrible );
VIDEO_START( ddrible );
VIDEO_UPDATE( ddrible );
