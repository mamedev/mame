/***************************************************************************

    Gaelco game hardware from 1991-1996

***************************************************************************/

typedef struct _gaelco_state gaelco_state;
struct _gaelco_state
{
	/* memory pointers */
	UINT16 *     videoram;
	UINT16 *     spriteram;
	UINT16 *     vregs;
	UINT16 *     screen;
//  UINT16 *     paletteram;    // currently this uses generic palette handling

	/* video-related */
	tilemap      *tilemap[2];

	/* devices */
	const device_config *audiocpu;
};



/*----------- defined in video/gaelco.c -----------*/

WRITE16_HANDLER( gaelco_vram_w );

VIDEO_START( bigkarnk );
VIDEO_START( maniacsq );

VIDEO_UPDATE( bigkarnk );
VIDEO_UPDATE( maniacsq );
