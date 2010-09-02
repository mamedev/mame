/*************************************************************************

    Hot Pinball
    Gals Pinball

*************************************************************************/

class galspnbl_state : public driver_device
{
public:
	galspnbl_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT16 *    videoram;
	UINT16 *    bgvideoram;
	UINT16 *    colorram;
	UINT16 *    scroll;
	UINT16 *    spriteram;
//  UINT16 *    paletteram; // currently this uses generic palette handling
	size_t      spriteram_size;

	/* devices */
	running_device *audiocpu;
};


/*----------- defined in video/galspnbl.c -----------*/


PALETTE_INIT( galspnbl );
VIDEO_UPDATE( galspnbl );
