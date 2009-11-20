/*************************************************************************

	Zero Zone

*************************************************************************/

typedef struct _zerozone_state zerozone_state;
struct _zerozone_state
{
	/* memory pointers */
	UINT16 *    videoram;
//	UINT16 *    paletteram16;	// currently this uses generic palette handling

	/* video-related */
	UINT16      tilebank;
	tilemap     *zz_tilemap;
};

WRITE16_HANDLER( zerozone_tilemap_w );
WRITE16_HANDLER( zerozone_tilebank_w );

VIDEO_START( zerozone );
VIDEO_UPDATE( zerozone );
