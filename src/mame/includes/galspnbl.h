/*************************************************************************

    Hot Pinball
    Gals Pinball

*************************************************************************/

class galspnbl_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, galspnbl_state(machine)); }

	galspnbl_state(running_machine &machine)
		: driver_data_t(machine) { }

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
