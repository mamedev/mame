/*************************************************************************

    Taito Dual Screen Games

*************************************************************************/

class warriorb_state : public driver_device
{
public:
	warriorb_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT16 *   spriteram;
	size_t     spriteram_size;

	/* misc */
	INT32      banknum;
	int        pandata[4];

	/* devices */
	device_t *maincpu;
	device_t *audiocpu;
	device_t *tc0140syt;
	device_t *tc0100scn_1;
	device_t *tc0100scn_2;
	device_t *lscreen;
	device_t *rscreen;
	device_t *_2610_1l;
	device_t *_2610_1r;
	device_t *_2610_2l;
	device_t *_2610_2r;
};


/*----------- defined in video/warriorb.c -----------*/

VIDEO_START( warriorb );
VIDEO_UPDATE( warriorb );
