/*************************************************************************

    Zero Zone

*************************************************************************/

class zerozone_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, zerozone_state(machine)); }

	zerozone_state(running_machine &machine) { }

	/* memory pointers */
	UINT16 *    videoram;
//  UINT16 *    paletteram; // currently this uses generic palette handling
	size_t      videoram_size;

	/* video-related */
	UINT16      tilebank;
	tilemap_t     *zz_tilemap;

	/* devices */
	running_device *audiocpu;
};

/*----------- defined in video/zerozone.c -----------*/

WRITE16_HANDLER( zerozone_tilemap_w );
WRITE16_HANDLER( zerozone_tilebank_w );

VIDEO_START( zerozone );
VIDEO_UPDATE( zerozone );
