/*************************************************************************

    Mad Motor

*************************************************************************/

class madmotor_state : public driver_device
{
public:
	madmotor_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT16 *        spriteram;
//  UINT16 *        paletteram;     // this currently uses generic palette handlers
	size_t          spriteram_size;

	/* video-related */
	int             flipscreen;

	/* devices */
	device_t *maincpu;
	device_t *audiocpu;
};


/*----------- defined in video/madmotor.c -----------*/

VIDEO_START( madmotor );
SCREEN_UPDATE( madmotor );
