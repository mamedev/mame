/*************************************************************************

    Funky Jet

*************************************************************************/

class funkyjet_state : public driver_device
{
public:
	funkyjet_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT16 *  pf1_rowscroll;
	UINT16 *  pf2_rowscroll;
	UINT16 *  spriteram;
//  UINT16 *  paletteram;    // currently this uses generic palette handling (in deco16ic.c)
	size_t    spriteram_size;

	/* devices */
	device_t *maincpu;
	device_t *audiocpu;
	device_t *deco16ic;
};



/*----------- defined in video/funkyjet.c -----------*/

VIDEO_UPDATE( funkyjet );
