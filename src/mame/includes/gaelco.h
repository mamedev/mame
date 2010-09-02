/***************************************************************************

    Gaelco game hardware from 1991-1996

***************************************************************************/

class gaelco_state : public driver_device
{
public:
	gaelco_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

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
