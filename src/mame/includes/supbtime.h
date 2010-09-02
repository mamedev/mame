/*************************************************************************

    Super Burger Time & China Town

*************************************************************************/

class supbtime_state : public driver_device
{
public:
	supbtime_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT16 *  pf1_rowscroll;
	UINT16 *  pf2_rowscroll;
	UINT16 *  spriteram;
//  UINT16 *  paletteram;    // currently this uses generic palette handling (in deco16ic.c)
	size_t    spriteram_size;

	/* video-related */

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
	running_device *deco16ic;
};



/*----------- defined in video/supbtime.c -----------*/

VIDEO_UPDATE( supbtime );
