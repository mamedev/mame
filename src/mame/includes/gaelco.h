/***************************************************************************

    Gaelco game hardware from 1991-1996

***************************************************************************/

class gaelco_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, gaelco_state(machine)); }

	gaelco_state(running_machine &machine) { }

	/* memory pointers */
	UINT16 *     videoram;
	UINT16 *     spriteram;
	UINT16 *     vregs;
	UINT16 *     screen;
//  UINT16 *     paletteram;    // currently this uses generic palette handling

	/* video-related */
	tilemap_t      *tilemap[2];

	/* devices */
	running_device *audiocpu;
};



/*----------- defined in video/gaelco.c -----------*/

WRITE16_HANDLER( gaelco_vram_w );

VIDEO_START( bigkarnk );
VIDEO_START( maniacsq );

VIDEO_UPDATE( bigkarnk );
VIDEO_UPDATE( maniacsq );
