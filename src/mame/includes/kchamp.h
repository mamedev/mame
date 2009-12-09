/*************************************************************************

    Karate Champ

*************************************************************************/

typedef struct _kchamp_state kchamp_state;
struct _kchamp_state
{
	/* memory pointers */
	UINT8 *    videoram;
	UINT8 *    colorram;
	UINT8 *    spriteram;
	size_t     spriteram_size;

	/* video-related */
	tilemap    *bg_tilemap;

	/* misc */
	int        nmi_enable;
	int        sound_nmi_enable;
	int        msm_data;
	int        msm_play_lo_nibble;
	int        counter;

	/* devices */
	const device_config *audiocpu;
};


/*----------- defined in video/kchamp.c -----------*/

WRITE8_HANDLER( kchamp_videoram_w );
WRITE8_HANDLER( kchamp_colorram_w );
WRITE8_HANDLER( kchamp_flipscreen_w );

PALETTE_INIT( kchamp );
VIDEO_START( kchamp );
VIDEO_UPDATE( kchamp );
VIDEO_UPDATE( kchampvs );
