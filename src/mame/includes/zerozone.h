/*************************************************************************

    Zero Zone

*************************************************************************/

typedef struct _zerozone_state zerozone_state;
struct _zerozone_state
{
	/* memory pointers */
	UINT16 *    videoram;
//  UINT16 *    paletteram; // currently this uses generic palette handling
	size_t      videoram_size;

	/* video-related */
	UINT16      tilebank;
	tilemap     *zz_tilemap;

	/* devices */
	const device_config *audiocpu;
};

/*----------- defined in video/zerozone.c -----------*/

WRITE16_HANDLER( zerozone_tilemap_w );
WRITE16_HANDLER( zerozone_tilebank_w );

VIDEO_START( zerozone );
VIDEO_UPDATE( zerozone );
