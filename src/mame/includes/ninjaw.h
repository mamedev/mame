/*************************************************************************

    Taito Triple Screen Games

*************************************************************************/

class ninjaw_state : public driver_device
{
public:
	ninjaw_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT16 *   spriteram;
	size_t     spriteram_size;

	/* misc */
	UINT16     cpua_ctrl;
	INT32      banknum;
	int        pandata[4];

	/* devices */
	device_t *maincpu;
	device_t *audiocpu;
	device_t *subcpu;
	device_t *tc0140syt;
	device_t *tc0100scn_1;
	device_t *tc0100scn_2;
	device_t *tc0100scn_3;
	device_t *lscreen;
	device_t *mscreen;
	device_t *rscreen;
	device_t *_2610_1l;
	device_t *_2610_1r;
	device_t *_2610_2l;
	device_t *_2610_2r;
};


/*----------- defined in video/ninjaw.c -----------*/

VIDEO_START( ninjaw );
VIDEO_UPDATE( ninjaw );
